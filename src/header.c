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

void printint(int x) {printf("%d\n", x);}
void printfloat(double x) {printf("%g\n", x);}
void printbool(bool x) {printf("%s\n", x ? MORPHO_TRUESTRING : MORPHO_FALSESTRING);}
void printerr(char x[]) {fprintf(stderr, "%s\n", x);}
void printnil() {printf("MORPHO_NILSTRING\n");}
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
    exit(1);
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
    exit(1);
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
    exit(1);
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
    exit(1);
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
    exit(1);
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
    exit(1);
}
