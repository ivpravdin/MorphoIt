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

void or_backtrack(userfn_sig thisfn_sig, const varray_instruction *code, userfn_map &userfn_asts, bool is_main) {
    try {
        generate_userfn_ast(thisfn_sig, code, userfn_asts, is_main);
        return;
    } catch (const userfn_sig &specialized_sig) {
        std::cerr << "[backtracking...]\n";
        // main is never recursive so we know this will never be a call to main
        or_backtrack(specialized_sig, code, userfn_asts, false); 
        or_backtrack(thisfn_sig, code, userfn_asts, is_main); 
    }
}

void generate_all_userfn_asts(
    const objectfunction *globalfn_obj,
    const varray_instruction *code,
    userfn_map &userfn_asts
) {
    const varray_value const_table = globalfn_obj->konst;
    size_t nconsts = const_table.count;
    userfn_sig toplevel_sig = { .objfn = globalfn_obj };

    size_t i = 0;
    or_backtrack(userfn_sig { .objfn = globalfn_obj }, code, userfn_asts, true);
    fprintf(stderr, "[Toplevel section finished!]\n");


    std::cerr << "[Subfunctions...]\n";
    for (size_t i = 0; i < nconsts; i++) {
        if (!MORPHO_ISFUNCTION(const_table.data[i])) continue;

        userfn_sig sig = { .objfn = MORPHO_GETFUNCTION(const_table.data[i]) };
        or_backtrack(sig, code, userfn_asts, false);
    }
}