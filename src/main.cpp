#include <iostream>
#include <fstream>
#include <iterator>
#include <filesystem>
#include <cstring>

#include "blocks/c_code_generator.h"
#include "builder/static_var.h"
#include "builder/dyn_var.h"

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

#include "morpho_header.h"

#include "value.h"
#include "builtin.h"

static dyn_var<int> morpho_vm(const int n, const uint32_t instructions[], objectfunction *globalfn) {
    dyn_var<value[255]> reg;
    dyn_var<value> left, right;
    
    dyn_var<value[100]> globals;
    
    static_var<int32_t> a, b, c;
    static_var<int32_t> bc;
    static_var<int32_t> pc = 0;

    while (pc < n) {
        bc = instructions[pc];
        switch (DECODE_OP(bc)) {
            /* OPCODE: NO-OP
             * [COMPLETE]
             */
            case OP_NOP:
                break;

            case OP_MOV:
                a=DECODE_A(bc); b=DECODE_B(bc);
                reg[a] = reg[b];
                break;

            case OP_LCT:
                a=DECODE_A(bc); b=DECODE_Bx(bc);
                reg[a] = globalfn->konst.data[b];
                break;
            /* OPCODE: ADD
             * 
             * To-Do:
             *      - Finish string concatenation?
             *      - Object operator redirection
             */
            case OP_ADD:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b], right = reg[c];
                if (MORPHO_ISFLOAT(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) + X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) + X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                } else if (MORPHO_ISINTEGER(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) + X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) + X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                } else if (MORPHO_ISSTRING(left) && MORPHO_ISSTRING(right)) {
                    reg[a] = object_concatenatestring(left, right);
                }
                break;
            /* OPCODE: SUBTRACT
             * 
             * To-Do:
             *      - Object operator redirection
             */
            case OP_SUB:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b], right = reg[c];

                if (MORPHO_ISFLOAT(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) - X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) - X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                } else if (MORPHO_ISINTEGER(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) - X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) - X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                }
                break;
            /* OPCODE: MULTIPLY
             * 
             * To-Do:
             *      - Object operator redirection
             */
            case OP_MUL:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b], right = reg[c];

                if (MORPHO_ISFLOAT(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) * X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) * X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                } else if (MORPHO_ISINTEGER(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) * X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) * X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                }
                break;
            /* OPCODE: DIVIDE
             * 
             * To-Do:
             *      - Object operator redirection
             */
            case OP_DIV:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b], right = reg[c];

                if (MORPHO_ISFLOAT(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) / X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) / X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                } else if (MORPHO_ISINTEGER(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) / X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) / X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                }
                break;
            /* OPCODE: POW
             * 
             * To-Do:
             *      - Verify implementation?
             *      - Object operator redirection
             */
            case OP_POW: //POW
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b];
                right = reg[c];

                if (MORPHO_ISFLOAT(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( runtime::pow(X_MORPHO_GETFLOATVALUE(left), X_MORPHO_GETFLOATVALUE(right)) );
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_FLOAT( runtime::pow(X_MORPHO_GETFLOATVALUE(left), S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(right)) ));
                        break;
                    }
                } else if (MORPHO_ISINTEGER(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( runtime::pow(S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(left)),
                                                            X_MORPHO_GETFLOATVALUE(right)) );
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = 
                            X_MORPHO_FLOAT( runtime::pow(S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(left)),
                                                        S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(right))));
                        break;
                    }
                }

                // OPREDIRECT(powselector, powrselector, a);
                // OPERROR("Exponentiate")
                break;
            case OP_EQ:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(!x_morpho_extendedcomparevalue(left, right));
                break;
            case OP_NEQ:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(x_morpho_extendedcomparevalue(left, right));
                break;
            case OP_NOT:
                a=DECODE_A(bc); b=DECODE_B(bc);
                left = reg[b];

                if (MORPHO_ISBOOL(left)) {
                    reg[a] = X_MORPHO_BOOL(!X_MORPHO_GETBOOLVALUE(left));
                } else {
                    reg[a] = X_MORPHO_BOOL(MORPHO_ISNIL(left));
                }
                break;
            case OP_LT: //LT
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b];
                right = reg[c];

                reg[a] = (left<right);
                break;
            case OP_B: // B
                b=DECODE_sBx(bc);
                pc+=b;
                break;
            case OP_BIFF: // BIFF
                a=DECODE_A(bc);
                left=reg[a];

                if (!left) pc+=DECODE_sBx(bc);
                break;
            case OP_LGL: // LGL
                a=DECODE_A(bc);
                b=DECODE_Bx(bc);
                reg[a]=globals[b];
                break;
            case OP_SGL: // SGL
                a=DECODE_A(bc);
                b=DECODE_Bx(bc);
                globals[b]=reg[a];
                break;
            case OP_PRINT: // PRINT
                a=DECODE_A(bc);
                left=reg[a];
                if (MORPHO_ISINTEGER(left)) {
                    runtime::printint( X_MORPHO_GETINTEGERVALUE(left) );
                } else if (MORPHO_ISFLOAT(left)) {
                    runtime::printfloat( X_MORPHO_GETFLOATVALUE(left) );
                } else if (MORPHO_ISBOOL(left)) {
                    runtime::printbool(X_MORPHO_GETBOOLVALUE(left));
                } else if (MORPHO_ISNIL(left)) {
                    runtime::printnil();
                } else {
                    runtime::printerr("Print not implemented for this type.");
                }
                break;
            case OP_END: // END
                return EXIT_SUCCESS;
            default:
                runtime::printerr("Encountered unimplemented instruction. Exiting.");
                return EXIT_FAILURE;
        }

        pc++;
    }
    return EXIT_FAILURE;
}

int main(int argc, char* argv[]) {
    bool hexdump = false;
    char *src_file_path = NULL;
    if (argc > 3 or argc == 1) {
        std::cerr << "Usage: " 
             << std::filesystem::path(argv[0]).filename().string() 
             << " [-D] MORPHO_FILE\n" 
             << "(use '-' for stdin)"
             << std::endl;
        return EXIT_FAILURE;
    } else if (argc == 3 and strcmp(argv[1], "-D") == 0) {
        hexdump = true;
        src_file_path = argv[2];
    }
    else {
        src_file_path = argv[1];
    }

    assert(src_file_path);

    std::istream *input = &std::cin;
    std::ifstream file;
    if (strcmp(src_file_path, "-") != 0) {
        file = std::ifstream(src_file_path, std::ios::in);
        if (not file.is_open()) {
            std::cerr << "Could not open '" << src_file_path << "'. Exiting." << std::endl;
            return EXIT_FAILURE;
        }
        input = &file;
    }

    std::string src(std::istreambuf_iterator<char>{*input}, {});

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

        if (hexdump) {
            for (int i = 0; i < ninstructions; i++) {
                std::cout << "0x"
                     << std::setfill('0')
                     << std::setw(sizeof(*bytecode) * 2)
                     << std::hex
                     << bytecode[i] << "\n";
            }
        }
        else {
            auto ast = context.extract_function_ast(morpho_vm, "main", ninstructions, bytecode, globalfn);
            print_wrapper_code(std::cout);
            block::c_code_generator::generate_code(ast, std::cout, 0);
        }

    } else {
        fprintf(stderr, "Morpho compilation error [%s]: %s\n", err.id, err.msg);
        return EXIT_FAILURE;
    }

    morpho_freeprogram(p);
    morpho_freecompiler(c);

    morpho_finalize();

	return EXIT_SUCCESS;
}
