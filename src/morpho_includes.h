#ifndef _MORPHO_INCLUDES_H_
#define _MORPHO_INCLUDES_H_

// #ifdef _WIN32
// #define MorphoThread HANDLE
// #define MorphoMutex CRITICAL_SECTION
// #else
// #define MorphoThread pthread_t
// #define MorphoMutex pthread_mutex_t
// #endif

extern "C"
{
    // the actual platform.h uses vendored C complex number implementations
    // which is incompatible with C++, or at least
    #include "platform.h"
    #include <morpho/vm.h>
    #include <morpho/classes.h>
    #include <morpho/compile.h>
    #include <morpho/profile.h>
    #include <morpho/program.h>
    #include <morpho/morpho.h>

    // Prototypes for a couple of 
    varray_instruction *program_getbytecode(program *p);
    objectfunction *program_getglobalfn(program *p);
}

#endif // _MORPHO_INCLUDES_H_