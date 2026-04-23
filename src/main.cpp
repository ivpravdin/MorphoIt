#include <iostream>
#include <fstream>
#include <iterator>
#include <filesystem>
#include <cstring>
#include <dlfcn.h>
#include <map>

#include "blocks/c_code_generator.h"

// Include the BuildIt types

#include "morpho_header.h"
#include "pe_vm.h"
#include "runtime.h"
#include "pe_vm_consts.h"

#define TMP_C_FILE "/tmp/pe_out.c"

void* compile_and_load_lib(const char* filepath) {
    char cmd[256];
#ifdef DEBUG
    snprintf(cmd, sizeof(cmd), "cc -g -fno-omit-frame-pointer -shared -fPIC -lmorpho -L/usr/local/lib -I/usr/local/include -Wno-incompatible-pointer-types -Wno-int-conversion %s -o /tmp/pe_out.so", filepath);
#else
    snprintf(cmd, sizeof(cmd), "cc -O3 -shared -fPIC -L/usr/local/lib -I/usr/local/include -lmorpho -Wno-incompatible-pointer-types -Wno-int-conversion %s -o /tmp/pe_out.so", filepath);
#endif
    int rc = system(cmd);
    if (rc != 0) {
        std::cerr << "Compilation failed\n";
        exit(1);
    }

    void* lib = dlopen("/tmp/pe_out.so", RTLD_NOW);
    if (!lib) {
        std::cerr << "dlopen failed: " << dlerror() << "\n";
        exit(1);
}

return lib;
}

void generate_userfn_asts(
    const varray_instruction &code,
    const varray_value &const_table,
    std::map<uintptr_t, block::stmt::Ptr> &userfn_asts
) {
    const size_t ninstructions = code.count;
    const instruction *bytecode = (instruction *) code.data;
    const size_t nconsts = const_table.count;

    for (size_t i = 0; i < nconsts; i++) {
        if (MORPHO_ISFUNCTION(const_table.data[i])) {
            objectfunction *fn = MORPHO_GETFUNCTION(const_table.data[i]);
            uintptr_t fnptr = (uintptr_t) fn;

            if (userfn_asts.count(fnptr) > 0) break;

            std::cerr << "[PE'ing..." <<  get_mangled_fn_name(fn) << "]\n";

            builder::builder_context ctxt;
            auto ast = ctxt.extract_function_ast(
                morpho_vm,
                    get_mangled_fn_name(fn),
                    ninstructions,
                    bytecode,
                    fn
            );
            auto [_, was_inserted] = userfn_asts.insert_or_assign(fnptr, std::move(ast));
            assert(was_inserted);

            generate_userfn_asts(code, fn->konst, userfn_asts);
        }
    }
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
    std::ifstream src_file;
    if (strcmp(src_file_path, "-") != 0) {
        src_file = std::ifstream(src_file_path, std::ios::in);
        if (not src_file.is_open()) {
            std::cerr << "Could not open '" << src_file_path << "'. Exiting." << std::endl;
            return EXIT_FAILURE;
        }
        input = &src_file;
    }

    std::string src(std::istreambuf_iterator<char>{*input}, {});
    src_file.close();

	builder::builder_context context;

    morpho_initialize();

    program *p = morpho_newprogram();
    compiler *c = morpho_newcompiler(p);

    error err;
    error_init(&err);

    if (!morpho_compile(const_cast<char*>(src.c_str()), c, false, &err)) {
        fprintf(stderr, "Morpho compilation error [%s]: %s\n", err.id, err.msg);
        morpho_freeprogram(p);
        morpho_freecompiler(c);
        morpho_finalize();
        return EXIT_FAILURE;
    }

    varray_instruction *code = program_getbytecode(p);
    objectfunction *globalfn = program_getglobalfn(p);

    instruction *bytecode = (instruction *) code->data;
    int ninstructions = code->count;

    if (hexdump) {
        for (int i = 0; i < ninstructions; i++) {
            std::cout << "0x"
                    << std::setfill('0')
                    << std::setw(sizeof(*bytecode) * 2)
                    << std::hex
                    << bytecode[i] << "\n";
        }

        morpho_freeprogram(p);
        morpho_freecompiler(c);
        morpho_finalize();
        return EXIT_SUCCESS;
    }

    std::ofstream out_c_file(TMP_C_FILE);


    std::map<uintptr_t, block::stmt::Ptr> userfn_asts;
    block::stmt::Ptr ast;

    try {
        generate_userfn_asts(*code, globalfn->konst, userfn_asts);
        std::cerr << "[PE'ing main]\n";
        ast = context.extract_function_ast(
            morpho_vm,
            "main_morpho",
                ninstructions,
                bytecode,
                globalfn
        );
    } catch (std::exception &e) {
        std::cerr << "Partial evaluation exited early with error: '"
                  << e.what()
                  << "'\n";
        morpho_freeprogram(p);
        morpho_freecompiler(c);
        morpho_finalize();
        return EXIT_FAILURE;
    }

    print_wrapper_code(out_c_file);
    std::cerr << "[CODE GENERATED]\n";

    // user fn declarations
    for (auto [key, ast] : userfn_asts) {
        block::c_code_generator::generate_code(ast, out_c_file, 0, true);
        out_c_file << generate_fnobj_definition((objectfunction *) key);
    }
    // user fn definitions
    for (auto [key, ast] : userfn_asts) {
        block::c_code_generator::generate_code(ast, out_c_file, 0);
    }

    // main
    block::c_code_generator::generate_code(ast, out_c_file, 0);
    out_c_file.close();

    std::cerr << "[COMPILING]\n";
    void* lib = compile_and_load_lib(TMP_C_FILE);
    userfn main_morpho = reinterpret_cast<userfn>(dlsym(lib, "main_morpho"));
    std::cerr << "[RUNNING]\n";
    int result = main_morpho(NULL);
    //std::cout << "Program exited with code " << result << std::endl;


    morpho_freeprogram(p);
    morpho_freecompiler(c);
    morpho_finalize();

	return EXIT_SUCCESS;
}
