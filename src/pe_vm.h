#ifndef PE_VM_H
#define PE_VM_H

#include "builder/dyn_var.h"

#include "morpho_header.h"

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

/** This structure will contain any persistent state associated with execution. */
typedef struct {
   dyn_var<value[100]> globals; 
} vm_t;

dyn_var<value> vm_enter(const int n, const uint32_t instructions[], objectfunction *fn);
dyn_var<value> morpho_vm(vm_t *v, const int n, const uint32_t instructions[], objectfunction *fn);

#endif // PE_VM_H