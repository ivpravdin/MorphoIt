#ifndef PE_VM_H
#define PE_VM_H

#include <optional>
#include <variant>
#include "builder/dyn_var.h"
#include "blocks/c_code_generator.h"

#include "value.h"
#include "morpho_header.h"
#include "userfn.h"


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


builder::dyn_var<value> morpho_vm(
    builder::dyn_var<value *> args,
    const int n,
    const instruction *const instructions,
    const userfn_sig &thisfn_sig,
    userfn_details &thisfn_deets,
    userfn_map &subfn_asts,
    const bool is_main
);

#endif // PE_VM_H