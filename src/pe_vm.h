#ifndef PE_VM_H
#define PE_VM_H

#include "builder/dyn_var.h"
#include "blocks/c_code_generator.h"
#include <map>

#include "morpho_header.h"

builder::dyn_var<value> morpho_vm(
    builder::dyn_var<value *> args,
    const int n,
    const uint32_t *const instructions,
    const objectfunction *const globalfn,
    std::map<uintptr_t, block::stmt::Ptr> &subfn_asts
);

#endif // PE_VM_H