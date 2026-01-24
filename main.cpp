// Include the headers
#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include <iostream>
#include <fstream>
#include <iterator>
#include <filesystem>

extern "C"
{
  // Hackily include bits of morpho we need 
  #include "morpho.h"
  #define cmplx_h     // Suppress headers that don't play nice with C++
  #define platform_h  //
  #include "program.h"

  #include "varray.h" // Define a missing type that's protected from casual use
  typedef unsigned int instruction;
  DECLARE_VARRAY(instruction, instruction);

  // Prototypes for a couple of 
  varray_instruction *program_getbytecode(program *p);
  objectfunction *program_getglobalfn(program *p);
}

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

using namespace std;

/** Decode the opcode */
#define DECODE_OP(x) (x & 0xff)

/** Decode operand A */
#define DECODE_A(x) ((x>>8) & (0xff))
/** Decode operand B */
#define DECODE_B(x) ((x>>16) & (0xff))
/** Decode operand C */
#define DECODE_C(x) ((x>>24) & (0xff))

/** Decode long operand Bx */
#define DECODE_Bx(x) ((x>>16) & (0xffff))

/** Decode signed long operand Bx */
#define DECODE_sBx(x) ((short) ((x>>16) & (0xffff)))

dyn_var<void(int)> print(builder::as_global("print"));

static void print_wrapper_code(std::ostream &oss) {
	oss << "#include <stdio.h>\n";
	oss << "#include <stdlib.h>\n";
    oss << "#include <stdbool.h>\n";
	oss << "void print(int x) {printf(\"%d\\n\", x);}\n";
}

static dyn_var<int> morpho_vm(const int n, const uint32_t bytecode[], const uint32_t consts[]) {
    //static_var<uint32_t> consts[] = {0, 1, 30000};
    dyn_var<int32_t[255]> reg = {0};
    dyn_var<int32_t[255]> globals = {0};
    dyn_var<int32_t> left, right;
    
    static_var<int32_t> a, b, c;
    static_var<int32_t> bc;
    static_var<int32_t> pc = 0;

    while (pc < n) {
        bc = bytecode[pc];
        switch (DECODE_OP(bc)) {
            case 0x0: // NOP
                break;
            case 0x1: // MOV
                a=DECODE_A(bc); b=DECODE_B(bc);
                reg[a] = reg[b];
                break;
            case 0x2: // LCT
                a=DECODE_A(bc); b=DECODE_Bx(bc);
                reg[a] = consts[b];
                break;
            case 0x3: // ADD
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                reg[a] = reg[b] + reg[c];
                break;
            case 0xA: //LT
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b];
                right = reg[c];

                reg[a] = (left<right);
                break;
            case 0xD: // B
                b=DECODE_sBx(bc);
                pc+=b;
                break;
            case 0xF: // BIFF
                a=DECODE_A(bc);
                left=reg[a];

                if (!left) pc+=DECODE_sBx(bc);
                break;
            case 0x1D: // LGL
                a=DECODE_A(bc);
                b=DECODE_Bx(bc);
                reg[a]=globals[b];
                break;
            case 0x1E: // SGL
                a=DECODE_A(bc);
                b=DECODE_Bx(bc);
                globals[b]=reg[a];
                break;
            case 0x22: // PRINT
                a=DECODE_A(bc);
                left=reg[a];
                print(left);
                break;
            case 0x24: // END
                return 0;
            default:
                return 1;
        }

        pc++;
    }
    return 1;
}

int main(int argc, char* argv[]) {
    bool hexdump = false;
    char *src_file_path = NULL;
    if (argc > 3 or argc == 1) {
        cerr << "Usage: " 
             << filesystem::path(argv[0]).filename().string() 
             << " [-D] MORPHO_FILE" << endl;
        return EXIT_FAILURE;
    } else if (argc == 3 and strcmp(argv[1], "-D") == 0) {
        hexdump = true;
        src_file_path = argv[2];
    }
    else {
        src_file_path = argv[1];
    }

    assert(src_file_path);

    ifstream src_file(src_file_path, ios::in);
    if (not src_file.is_open()) {
        cerr << "Could not open '" << src_file_path << "'. Exiting." << endl;
        return EXIT_FAILURE;
    }

    string src(std::istreambuf_iterator<char>{src_file}, {});

	builder::builder_context context;

    morpho_initialize();

    program *p = morpho_newprogram();
    compiler *c = morpho_newcompiler(p);

    error err;
    error_init(&err);

    // that char * cast is to remove constness :P
    if (morpho_compile((char *)src.c_str(), c, false, &err)) {
        varray_instruction *code = program_getbytecode(p);
        objectfunction *globalfn = program_getglobalfn(p);
        
        uint32_t *bytecode = (uint32_t *) code->data;
        int ninstructions = code->count;

        int nconst = globalfn->konst.count; 
        uint32_t consts[255]; // Must be static in length
        for (int i=0; i<nconst; i++) {
            if (MORPHO_ISINTEGER(globalfn->konst.data[i])) {
                consts[i]=MORPHO_GETINTEGERVALUE(globalfn->konst.data[i]);
            } else {
                consts[i]=0;
                printf("Warning: constant %i '", i);
                morpho_printvalue(NULL, globalfn->konst.data[i]);
                printf("' will be ignored.\n");
            }
        }

        if (hexdump) {
            for (int i = 0; i < ninstructions; i++) {
                cout << "0x"
                     << std::setfill('0')
                     << std::setw(sizeof(*bytecode) * 2)
                     << std::hex
                     << bytecode[i] << "\n";
            }
        }
        else {
            auto ast = context.extract_function_ast(morpho_vm, "main", ninstructions, bytecode, consts);
            print_wrapper_code(std::cout);
            block::c_code_generator::generate_code(ast, std::cout, 0);
        }

    } else {
        printf("Compilation error [%s]: %s\n", err.id, err.msg);
    }

    morpho_freeprogram(p);
    morpho_freecompiler(c);

    morpho_finalize();

	return 0;
}
