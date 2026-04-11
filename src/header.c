#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <morpho/morpho.h>
#include <morpho/object.h>
#include <morpho/strng.h>
#include <morpho/builtin.h>
#include <morpho/metafunction.h>
#include <morpho/common.h>

// pe_vm_consts.h is inserted inline when runtime generates the header
// since it's not a system library header. This is here for IntelliSense
#ifndef PE_VM_CONSTS_H
#include "pe_vm_consts.h"
#endif

// this actually gets inserted by runtime.cpp
// this is just here for intellisense

value globals[PE_NUM_GLOBALS];

void printint(int32_t x) {printf("%d\n", x);}
void printfloat(double x) {printf("%g\n", x);}
void printbool(bool x) {printf("%s\n", x ? MORPHO_TRUESTRING : MORPHO_FALSESTRING);}
void printerr(char x[]) {fprintf(stderr, "%s\n", x);}
void printnil() {printf("%s\n", MORPHO_NILSTRING);}
void printstring(char *x) {printf("%s\n", x);}
void print(value val) {
    if (MORPHO_ISINTEGER(val)) {
        printint(MORPHO_GETINTEGERVALUE(val));
    } else if (MORPHO_ISFLOAT(val)) {
        printfloat(MORPHO_GETFLOATVALUE(val));
    } else if (MORPHO_ISBOOL(val)) {
        printbool(MORPHO_GETBOOLVALUE(val));
    } else if (MORPHO_ISNIL(val)) {
        printnil();
    } else if (MORPHO_ISSTRING(val)) {
        printstring(MORPHO_GETCSTRING(val));
    } else if (MORPHO_ISOBJECT(val)) {
        object_print(0, MORPHO_GETOBJECT(val));
        printstring("");
    } else {
        printerr("Unknown type to print");
    }
}

inline value op_add(value a, value b) {
    if (MORPHO_ISINTEGER(a)) {
        if (MORPHO_ISINTEGER(b)) {
            return MORPHO_INTEGER(MORPHO_GETINTEGERVALUE(a) + MORPHO_GETINTEGERVALUE(b));
        } else if (MORPHO_ISFLOAT(b)) {
            return MORPHO_FLOAT(MORPHO_GETINTEGERVALUE(a) + MORPHO_GETFLOATVALUE(b));
        }
    } else if (MORPHO_ISFLOAT(a)) {
        if (MORPHO_ISINTEGER(b)) {
            return MORPHO_FLOAT(MORPHO_GETFLOATVALUE(a) + MORPHO_GETINTEGERVALUE(b));
        } else if (MORPHO_ISFLOAT(b)) {
            return MORPHO_FLOAT(MORPHO_GETFLOATVALUE(a) + MORPHO_GETFLOATVALUE(b));
        }
    } else if (MORPHO_ISSTRING(a)) {
        if (MORPHO_ISSTRING(b)) {
            return object_concatenatestring(a, b);
        }
    }
    printerr("Unsupported types for addition");
    exit(EXIT_FAILURE);
}

inline value op_sub(value left, value right) {
    if (MORPHO_ISFLOAT(left)) {
        if (MORPHO_ISFLOAT(right)) {
            return MORPHO_FLOAT( MORPHO_GETFLOATVALUE(left) - MORPHO_GETFLOATVALUE(right));
        } else if (MORPHO_ISINTEGER(right)) {
            return MORPHO_FLOAT( MORPHO_GETFLOATVALUE(left) - MORPHO_GETINTEGERVALUE(right));
        }
    } else if (MORPHO_ISINTEGER(left)) {
        if (MORPHO_ISFLOAT(right)) {
            return MORPHO_FLOAT( MORPHO_GETINTEGERVALUE(left) - MORPHO_GETFLOATVALUE(right));
        } else if (MORPHO_ISINTEGER(right)) {
            return MORPHO_INTEGER( MORPHO_GETINTEGERVALUE(left) - MORPHO_GETINTEGERVALUE(right));
        }
    }

    // type error...
    printerr("Unsupported types for sub");
    exit(EXIT_FAILURE);
}

inline value op_mul(value left, value right) {
    if (MORPHO_ISFLOAT(left)) {
        if (MORPHO_ISFLOAT(right)) {
            return MORPHO_FLOAT( MORPHO_GETFLOATVALUE(left) * MORPHO_GETFLOATVALUE(right));
        } else if (MORPHO_ISINTEGER(right)) {
            return MORPHO_FLOAT( MORPHO_GETFLOATVALUE(left) * MORPHO_GETINTEGERVALUE(right));
        }
    } else if (MORPHO_ISINTEGER(left)) {
        if (MORPHO_ISFLOAT(right)) {
            return MORPHO_FLOAT( MORPHO_GETINTEGERVALUE(left) * MORPHO_GETFLOATVALUE(right));
        } else if (MORPHO_ISINTEGER(right)) {
            return MORPHO_INTEGER( MORPHO_GETINTEGERVALUE(left) * MORPHO_GETINTEGERVALUE(right));
        }
    }

    // type error...
    printerr("Unsupported types for mul");
    exit(EXIT_FAILURE);
}

inline value op_div(value left, value right) {
    if (MORPHO_ISFLOAT(left)) {
        if (MORPHO_ISFLOAT(right))
            return MORPHO_FLOAT(MORPHO_GETFLOATVALUE(left) /
                                    MORPHO_GETFLOATVALUE(right));
        else if (MORPHO_ISINTEGER(right))
            return MORPHO_FLOAT(MORPHO_GETFLOATVALUE(left) /
                                    (double) MORPHO_GETINTEGERVALUE(right));
        // else // (right_type is DYN) ...
    } else if (MORPHO_ISINTEGER(left)) {
        if (MORPHO_ISFLOAT(right))
            return MORPHO_FLOAT((double) MORPHO_GETINTEGERVALUE(left) /
                                    MORPHO_GETFLOATVALUE(right));
        else if (MORPHO_ISINTEGER(right))
            return MORPHO_FLOAT((double) MORPHO_GETINTEGERVALUE(left) /
                                    (double) MORPHO_GETINTEGERVALUE(right));
        // else // right_type is DYN
    }
    printerr("Unsupported types for div");
    exit(EXIT_FAILURE);
}

inline value op_pow(value left, value right) {
    if (MORPHO_ISFLOAT(left)) {
        if (MORPHO_ISFLOAT(right)) {
            return MORPHO_FLOAT( pow(MORPHO_GETFLOATVALUE(left), 
                                                MORPHO_GETFLOATVALUE(right)) );
        } else if (MORPHO_ISINTEGER(right)) {
            return MORPHO_FLOAT( pow(MORPHO_GETFLOATVALUE(left),
                                                (double) MORPHO_GETINTEGERVALUE(right)) );
        }
    } else if (MORPHO_ISINTEGER(left)) {
        if (MORPHO_ISFLOAT(right)) {
            return MORPHO_FLOAT( pow((double) MORPHO_GETINTEGERVALUE(left),
                                                MORPHO_GETFLOATVALUE(right) ));
        } else if (MORPHO_ISINTEGER(right)) {
            return MORPHO_FLOAT( pow((double) MORPHO_GETINTEGERVALUE(left),
                                                (double) MORPHO_GETINTEGERVALUE(right)));
        }
    }

    // type error
    printerr("Unsupported types for pow");
    exit(EXIT_FAILURE);
}

inline value op_not(value left) {

    if (MORPHO_ISBOOL(left)) {
        return MORPHO_BOOL(!MORPHO_GETBOOLVALUE(left));
    } else {
        return MORPHO_BOOL(MORPHO_ISNIL(left));
    }
}

typedef value (*userfn)(const value const*, int32_t, int32_t);
inline value call_userfn(const value *const reg, int32_t a, int32_t b, int32_t c) {
    // GETOBJECT just unwraps a pointer, really, but instead of an obj here
    // it's a ptr to a function. Or should be if we load the register correctly
    userfn f = *((userfn*)(MORPHO_GETOBJECT(reg[a])));
    const value *const args = reg + a;
    int n_pos_args = b;
    int n_opt_args = c;
    return f(args, n_pos_args, n_opt_args);
}


inline value call(value func, int nargs, value *args) {
    if (MORPHO_ISMETAFUNCTION(func)) {
        metafunction_resolve(MORPHO_GETMETAFUNCTION(func), nargs, args + 1, NULL, &func);
    }
    if (MORPHO_ISBUILTINFUNCTION(func)) {
        objectbuiltinfunction *f = MORPHO_GETBUILTINFUNCTION(func);
        return (f->function)(NULL, nargs, args);
    }
    printf("Attempted to call function of type: %lld != %d", MORPHO_GETTYPE(func), OBJECT_BUILTINFUNCTION);
    exit(EXIT_FAILURE);
}

