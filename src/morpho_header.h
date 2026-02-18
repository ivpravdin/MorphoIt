#ifndef SRC_MORPHO_HEADER_H
#define SRC_MORPHO_HEADER_H

extern "C"
{
    #ifdef _WIN32
    #define MorphoThread HANDLE
    #define MorphoMutex CRITICAL_SECTION
    #else
    #define MorphoThread pthread_t
    #define MorphoMutex pthread_mutex_t
    #endif
    #define cmplx_h
    #define platform_h
    #include <vm.h>
    #include <classes.h>
    #include <compile.h>
    #include <profile.h>
    #include <program.h>
    #include <morpho.h>

    // Prototypes for a couple of 
    varray_instruction *program_getbytecode(program *p);
    objectfunction *program_getglobalfn(program *p);
}

#endif // SRC_MORPHO_HEADER_H