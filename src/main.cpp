#include <iostream>
#include <fstream>
#include <iterator>
#include <filesystem>
#include <cstring>

#include "blocks/c_code_generator.h"

// Include the BuildIt types

#include "morpho_header.h"
#include "pe_vm.h"
#include "builtin.h"

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

    if (morpho_compile(const_cast<char*>(src.c_str()), c, false, &err)) {
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
