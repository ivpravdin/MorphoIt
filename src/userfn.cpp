#include "userfn.h"
#include "pe_vm.h"
#include "pe_vm_consts.h"

void generate_userfn_ast(
    const userfn_sig &sig,
    const varray_instruction *code,
    userfn_map &userfn_asts,
    bool is_main
) {
    // we only want to PE each function once.
    if (userfn_asts.count(sig) > 0 && userfn_asts.at(sig).has_been_evaluated()) {
        return;
    }

    const size_t ninstructions = code->count;
    const instruction *bytecode = reinterpret_cast<const instruction *>(code->data);

    userfn_details *deets_ptr = NULL;
    if (userfn_asts.count(sig) > 0) {
        deets_ptr = &userfn_asts.at(sig);
    } else {
        auto [deets_iter, was_inserted] = userfn_asts.emplace(
            sig,
            is_main ? userfn_details::with_name(PE_GLOBALFN_NAME)
                : userfn_details::from_sig(sig)
        );
        assert(was_inserted);
        deets_ptr = &deets_iter->second;
    }
    // as soon as a function is encountered, it must be registered in the map
    // whether or not it has been fully PE'd
    assert(deets_ptr);
    userfn_details &deets = *deets_ptr;

    fprintf(stderr, "[...PE'ing %s...]\n", deets.runtime_name.c_str());
    builder::builder_context ctxt;
    deets.fnast = ctxt.extract_function_ast(
        morpho_vm,
            deets.runtime_name,
            ninstructions,
            bytecode,
            sig,
            deets,
            userfn_asts,
            is_main
    );
    deets.mark_evaluated(); // critically important to avoid looping forever

    fprintf(stderr, "[...%s inserted!]\n", deets.runtime_name.c_str());
}

void generate_all_userfn_asts(
    const objectfunction *toplevel_fnobj,
    const varray_instruction *code,
    userfn_map &userfn_asts
) {
    const varray_value const_table = toplevel_fnobj->konst;
    size_t nconsts = const_table.count;
    userfn_sig toplevel_sig = { .objfn = toplevel_fnobj };

    size_t i = 0;
    while (true) {
        fprintf(stderr, "[%d'th try on toplevel function...]\n", ++i);
        try {
            generate_userfn_ast(toplevel_sig, code, userfn_asts, true);
            fprintf(stderr, "[Toplevel section finished!]\n");
            break;
        } catch (const userfn_sig &specialized_sig) {
            std::cerr << "[backtracking...]\n";
            // do the specialized called fn first
            // reason to organize this to do this first is to get return type
            generate_userfn_ast(specialized_sig, code, userfn_asts, false);
        }
    }


    std::cerr << "[Subfunctions...]\n";
    for (size_t i = 0; i < nconsts; i++) {
        if (!MORPHO_ISFUNCTION(const_table.data[i])) continue;

        userfn_sig sig = { .objfn = MORPHO_GETFUNCTION(const_table.data[i]) };
        size_t j = 0;
        while (true) {
            fprintf(stderr, "[%d'th try on subfn...]\n", ++j);
            try {
                generate_userfn_ast(sig, code, userfn_asts, false);
                break;
            } catch (userfn_sig &specialized_sig) {
                std::cerr << "[backtracking...]\n";
                generate_userfn_ast(specialized_sig, code, userfn_asts, false);
            }
        }
    }
}