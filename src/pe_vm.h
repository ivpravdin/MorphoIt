#ifndef PE_VM_H
#define PE_VM_H

#include <map>
#include <optional>
#include <variant>
#include "builder/dyn_var.h"
#include "blocks/c_code_generator.h"

#include "value.h"
#include "morpho_header.h"


class UnimplementedInstructionException: public std::exception
{
  virtual const char* what() const throw()
  {
    return "Unimplemented instruction encountered.";
  }
};

class OutOfBoundsPCException: public std::exception
{
  virtual const char* what() const throw()
  {
    return "Program counter exceeded bytecode buffer.";
  }
};

// class UpdatingSettledReturnTypeException: public std::exception
// {
//   virtual const char* what() const throw()
//   {
//     return "You can't do that.";
//   }
// };

struct userfn_sig {
    const objectfunction *objfn;
    std::optional<std::vector<pe_t_note>> argtypes = std::nullopt;

    // this exists just for std::map
    friend bool operator<(const userfn_sig &lhs, const userfn_sig &rhs){
        uintptr_t lobjfn = reinterpret_cast<uintptr_t>(lhs.objfn);
        uintptr_t robjfn = reinterpret_cast<uintptr_t>(rhs.objfn);
        return std::tie(lhs.objfn, lhs.argtypes) < std::tie(rhs.objfn, rhs.argtypes);
    }
};

// I'm doing this as a class ironically for readability. So it makes more sense
// how this thing is supposed to flow
class userfn_details {
public:
    block::stmt::Ptr fnast;

    pe_t_note get_returntype() {
        return return_type_state == userfn_details::EXPLORED ? returntype : pe_t_note::UNKNOWN;
    }

    // the explored state is really just for debugging, I suppose
    void mark_explored() {
        return_type_state = userfn_details::EXPLORED;
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
    enum {
        EXPLORED,
        EXPLORING,
        UNEXPLORED
    } return_type_state = UNEXPLORED;
    pe_t_note returntype = pe_t_note::UNKNOWN;
};

using userfn_map = std::map<userfn_sig, userfn_details>;

builder::dyn_var<value> morpho_vm(
    builder::dyn_var<value *> args,
    const int n,
    const instruction *const instructions,
    const userfn_sig &sig,
    userfn_map &subfn_asts,
    const bool is_main
);

#endif // PE_VM_H