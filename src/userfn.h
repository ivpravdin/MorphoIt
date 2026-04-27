#ifndef USERFN_H
#define USERFN_H

#include <optional>
#include <map>

#include "value.h"

constexpr const char USERFN_NAME_PREFIX[] = "morpho_userfn_";
constexpr const char USERFNOBJ_NAME_SUFFIX[] = "_wrapperstruct";

struct userfn_sig {
    const objectfunction *objfn;
    std::optional<std::vector<pe_t_note>> argtypes = std::nullopt;

    // this exists just for std::map
    friend bool operator<(const userfn_sig &lhs, const userfn_sig &rhs){
        uintptr_t lobjfn = reinterpret_cast<uintptr_t>(lhs.objfn);
        uintptr_t robjfn = reinterpret_cast<uintptr_t>(rhs.objfn);
        return std::tie(lhs.objfn, lhs.argtypes) < std::tie(rhs.objfn, rhs.argtypes);
    }

    std::string get_mangled_fn_name(bool genericize) const {
        std::string name = std::string(USERFN_NAME_PREFIX);
        if (MORPHO_ISOBJECT(this->objfn->name)) {
            name += MORPHO_GETCSTRING(this->objfn->name);
            name += "_";
        }
        name += std::to_string((uintptr_t) this->objfn);

        // use genericize to force to get generic name, even if is a specialized fn
        if (!genericize && this->argtypes.has_value()) {
            for (pe_t_note t : this->argtypes.value()) {
                name += "_" + gettypename(t);
            }
        }
        return std::move(name);
    }
};

// I'm doing this as a class ironically for readability. So it makes more sense
// how this thing is supposed to flow
class userfn_details {
public:
    block::stmt::Ptr fnast;
    std::string runtime_name;
    std::string runtime_generic_name;

    static userfn_details with_name(const std::string &name) {
        userfn_details deets;
        deets.runtime_name = name;
        deets.runtime_generic_name = name;
        return deets;
    }

    static userfn_details from_sig(const userfn_sig &sig) {
        userfn_details deets;
        deets.runtime_name = sig.get_mangled_fn_name(false);
        deets.runtime_generic_name = sig.get_mangled_fn_name(true);
        return deets;
    }

    pe_t_note get_returntype() const {
        return return_type_state == userfn_details::EXPLORED ? returntype : pe_t_note::UNKNOWN;
    }

    std::string get_runtime_fnobj_name(bool genericize) const {
        return this->get_runtime_fn_name(genericize) + USERFNOBJ_NAME_SUFFIX;
    }

    std::string get_runtime_fn_name(bool genericize) const {
        return (genericize ? runtime_generic_name : runtime_name);
    }

    void mark_evaluated() {
        return_type_state = userfn_details::EXPLORED;
        hasbeenevaluated = true;
    }

    bool has_been_evaluated() const {
        return hasbeenevaluated;
    }

    // We want the following invariant:
    // > If returntype is set to some T other than UNKNOWN then
    // > this function must ALWAYS return type T
    // What's going on here is that
    // we are fighting BuildIt's re-execution, which is why this update 
    // operation must be able to mutate the type from UNKNOWN to something else
    // once, and then any conflicting updates will cause it to 
    // go back to UNKNOWN and become idempotent
    void update_returntype(pe_t_note t) {
        switch (return_type_state)
        {
        case userfn_details::EXPLORED:
            std::cerr << "Warning: attempted to update return type of function that was marked explored. "
                         "You probably didn't want this.";
            return;

        case userfn_details::UNEXPLORED:
            returntype = t;
            return_type_state = userfn_details::EXPLORING;
            break;

        case userfn_details::EXPLORING:
            if (returntype != t) {
                returntype = pe_t_note::UNKNOWN;
            }
            break;
        }
    }

private:
    enum returntype_state {
        EXPLORED,
        EXPLORING,
        UNEXPLORED
    };
    enum returntype_state return_type_state = UNEXPLORED;
    bool hasbeenevaluated = false;
    pe_t_note returntype = pe_t_note::UNKNOWN;
};

using userfn_map = std::map<userfn_sig, userfn_details>;

void generate_userfn_ast(
    const userfn_sig &sig,
    const varray_instruction *code,
    userfn_map &userfn_asts,
    bool is_main
);

void generate_all_userfn_asts(
    const objectfunction *toplevel_fnobj,
    const varray_instruction *code,
    userfn_map &userfn_asts
);

#endif