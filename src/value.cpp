#include "value.h"
#include "morpho_header.h"
#include "builder/dyn_var.h"
#include "builtin.h"

using builder::dyn_var;
using builder::static_var;


DYNAMIFY_TYPE(bool) x_morpho_valuetofloat(DYNAMIFY_TYPE(value) v, DYNAMIFY_TYPE(double) *out) {
    if (MORPHO_ISINTEGER(v)) { *out = DYNAMIFY_TYPE(double) X_MORPHO_GETINTEGERVALUE(v); return true; }
    if (MORPHO_ISFLOAT(v)) { *out = X_MORPHO_GETFLOATVALUE(v); return true; }
    return false;
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
    DYNAMIFY_TYPE(double) absmax;
    if (absa>absb ? absa : absb);
    return (diff == 0.0) || (absmax > DBL_MIN && diff/absmax <= MORPHO_RELATIVE_EPS);
}

dyn_var<int> x_morpho_extendedcomparevalue(dyn_var<value> a, dyn_var<value> b, static_var<pe_t> atype, static_var<pe_t> btype) {
    if ((atype | btype) & PE_DYN_T) return runtime::morpho_extendedcomparevalue(a, b);

    if (atype == btype) return x_morpho_comparevalue(a, b, atype);

    // dyn_var<value> aa=a, bb=b;

    if (atype == PE_INT_T && btype == PE_FLOAT_T) {
        dyn_var<value> aa = X_MORPHO_INTEGERTOFLOAT(a);
        return x_morpho_comparevalue(aa, b, PE_FLOAT_T);
    } else if (atype == PE_FLOAT_T && btype == PE_INT_T) {
        dyn_var<value> bb = X_MORPHO_INTEGERTOFLOAT(b);
        return x_morpho_comparevalue(a, bb, PE_FLOAT_T);
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

dyn_var<int> x_morpho_comparevalue(dyn_var<value> a, dyn_var<value> b, static_var<pe_t> types) {
    //if (!morpho_ofsametype(a, b)) return MORPHO_NOTEQUAL;

    if (types == PE_FLOAT_T) {
        DYNAMIFY_TYPE(double) aa = X_MORPHO_GETFLOATVALUE(a);
        DYNAMIFY_TYPE(double) bb = X_MORPHO_GETFLOATVALUE(b);
        // if (x_morpho_doubleeqtest(aa, bb)) return MORPHO_EQUAL;
        // return bb > aa ? MORPHO_BIGGER : MORPHO_SMALLER;
        // TODO this is missing x_morpho_doubleeqtest but ffs
        // what is this: does the -1/0/1 for </=/> respectively in one line
        // why is this: i don't know but any flow control and you get missing
        // vars in produced code
        return !(aa == bb) * (-1 + 2*(bb > aa));
    }

    if (types == PE_NIL_T) {
        return MORPHO_EQUAL;
    }

    if (types == PE_INT_T) {
        dyn_var<int> aa = X_MORPHO_GETINTEGERVALUE(a);
        dyn_var<int> bb = X_MORPHO_GETINTEGERVALUE(b);
        return !(aa==bb) * (-1 + 2*(bb > aa));
        // if (aa==bb) return MORPHO_EQUAL;
        // return bb > aa ? MORPHO_BIGGER : MORPHO_SMALLER;
    }

    if (types == PE_BOOL_T) {
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