#include "value.h"
#include "morpho_header.h"
#include "builder/dyn_var.h"

using builder::dyn_var;
using builder::static_var;


DYNAMIFY_TYPE(bool) x_morpho_valuetofloat(DYNAMIFY_TYPE(value) v, DYNAMIFY_TYPE(double) *out) {
    if (MORPHO_ISINTEGER(v)) { *out = DYNAMIFY_TYPE(double) X_MORPHO_GETINTEGERVALUE(v); return true; }
    if (MORPHO_ISFLOAT(v)) { *out = X_MORPHO_GETFLOATVALUE(v); return true; }
    return false;
}

namespace runtime {
    DYNAMIFY_TYPE(double(double)) fabs(builder::as_global("fabs"));
}

// DYNAMIFY_TYPE(double) fabs(DYNAMIFY_TYPE(double) x) {
//     DYNAMIFY_TYPE(double) y = x;
//     if (y < 0) y = -y;
//     return y;
// }

/**  @brief Compare two doubles for equality using both absolute and relative tolerances */
DYNAMIFY_TYPE(bool) x_morpho_doubleeqtest(DYNAMIFY_TYPE(double) a, DYNAMIFY_TYPE(double) b) {
    if (a==b) return true; 
    DYNAMIFY_TYPE(double) diff = runtime::fabs(a-b);
    DYNAMIFY_TYPE(double) absa = runtime::fabs(a), absb=runtime::fabs(b);
    DYNAMIFY_TYPE(double) absmax = (absa>absb ? absa : absb);
    return (diff == 0.0) || (absmax > DBL_MIN && diff/absmax <= MORPHO_RELATIVE_EPS);
}

/** @brief Compares two values
 * @param a value to compare
 * @param b value to compare
 * @returns 0 if a and b are equal, a positive number if b\>a and a negative number if a\<b
 * @warning Requires that both values have the same type */
DYNAMIFY_TYPE(int) x_morpho_comparevalue(DYNAMIFY_TYPE(value) a, DYNAMIFY_TYPE(value) b) {
    //if (!morpho_ofsametype(a, b)) return MORPHO_NOTEQUAL;
    
    if (MORPHO_ISFLOAT(a)) {
        DYNAMIFY_TYPE(double) aa = X_MORPHO_GETFLOATVALUE(a);
        DYNAMIFY_TYPE(double) bb = X_MORPHO_GETFLOATVALUE(b);
        if (x_morpho_doubleeqtest(aa, bb)) return MORPHO_EQUAL;
        return (bb>aa ? MORPHO_BIGGER : MORPHO_SMALLER);
    }
    
    if (MORPHO_ISNIL(a)) {
        return MORPHO_EQUAL;
    }
    
    if (MORPHO_ISINTEGER(a)) {
        DYNAMIFY_TYPE(int) aa = X_MORPHO_GETINTEGERVALUE(a);
        DYNAMIFY_TYPE(int) bb = X_MORPHO_GETINTEGERVALUE(b);
        if (aa==bb) return MORPHO_EQUAL;
        return (bb>aa ? MORPHO_BIGGER : MORPHO_SMALLER);
    }
    
    if (MORPHO_ISBOOL(a)) {
        return (X_MORPHO_GETBOOLVALUE(b) != X_MORPHO_GETBOOLVALUE(a));
    }
    
    // if (MORPHO_ISOBJECT(a)) {
    //     if (X_MORPHO_GETOBJECTTYPE(a)!=X_MORPHO_GETOBJECTTYPE(b)) {
    //         return 1;
    //     }
    //     // TODO: write object_cmp for dyn_var
    //     //return object_cmp(X_MORPHO_GETOBJECT(a), X_MORPHO_GETOBJECT(b));
    // }
    
    return MORPHO_NOTEQUAL;
}



/** @brief Compares two values, even for inequivalent values e.g. int to float
 * @param a value to compare
 * @param b value to compare
 * @returns 0 if a and b are equal, a positive number if b\>a and a negative number if a\<b*/
DYNAMIFY_TYPE(int) x_morpho_extendedcomparevalue(DYNAMIFY_TYPE(value) a, DYNAMIFY_TYPE(value) b) {
    if (X_MORPHO_OFSAMETYPE(a, b)) return x_morpho_comparevalue(a, b);
    
    DYNAMIFY_TYPE(value) aa=a, bb=b;
    
    if (MORPHO_ISINTEGER(a) && MORPHO_ISFLOAT(b)) {
        aa = X_MORPHO_INTEGERTOFLOAT(a);
        return x_morpho_comparevalue(aa, bb);
    } else if (MORPHO_ISFLOAT(a) && MORPHO_ISINTEGER(b)) {
        bb = X_MORPHO_INTEGERTOFLOAT(b);
        return x_morpho_comparevalue(aa, bb);
    }

    // TODO: support complex numbers
    // } else if (MORPHO_ISCOMPLEX(bb) && MORPHO_ISNUMBER(aa)) {
    //     aa=b; bb=a;
    // }
    
    // if (X_MORPHO_ISCOMPLEX(aa) && X_MORPHO_ISNUMBER(bb)) {
    //     DYNAMIFY_TYPE(MorphoComplex) z = X_MORPHO_GETDOUBLECOMPLEX(aa);
    //     if (fabs(cimag(z)) < cabs(z)*MORPHO_RELATIVE_EPS) { // Ensure imaginary part is zero
    //         aa=X_MORPHO_FLOAT(creal(z));
    //         DYNAMIFY_TYPE(double) real;
    //         x_morpho_valuetofloat(bb, &real);
    //         bb=X_MORPHO_FLOAT(real);
    //     } else return MORPHO_NOTEQUAL;
    //     return x_morpho_comparevalue(aa, bb);
    // }
    
    return MORPHO_NOTEQUAL;
}