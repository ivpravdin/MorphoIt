#ifndef PE_VM_H
#define PE_VM_H

#include "builder/dyn_var.h"

#include "morpho_header.h"

builder::dyn_var<int> morpho_vm(const int n, const uint32_t instructions[], objectfunction *globalfn);

#endif // PE_VM_H