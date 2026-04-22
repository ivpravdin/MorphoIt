#include "value.h"
#include "morpho_header.h"
#include "builder/dyn_var.h"
#include "runtime.h"

using builder::dyn_var;
using builder::static_var;

pe_t_note gettypeannotation(value v) {
    switch (MORPHO_GETTYPE(v)) {
        case TAG_INT:  return pe_t_note::INT;
        case TAG_BOOL: return pe_t_note::BOOL;
        case TAG_NIL:  return pe_t_note::NIL;
        case TAG_OBJ:  return pe_t_note::OBJECT;
        default:       return pe_t_note::FLOAT;
    }
}

pe_t_note arith_binop_typerule(value t1, value t2) {
    if ((t1 != pe_t_note::FLOAT && t1 != pe_t_note::INT) || (t2 != pe_t_note::FLOAT && t2 != pe_t_note::INT))
        return pe_t_note::UNKNOWN;

    if (t1 == pe_t_note::FLOAT || t2 == pe_t_note::FLOAT)
        return pe_t_note::FLOAT;

    return pe_t_note::INT;
}

// DYNAMIFY_TYPE(bool) x_morpho_valuetofloat(DYNAMIFY_TYPE(value) v, DYNAMIFY_TYPE(double) *out) {
//     if (MORPHO_ISINTEGER(v)) { *out = DYNAMIFY_TYPE(double) X_MORPHO_GETINTEGERVALUE(v); return true; }
//     if (MORPHO_ISFLOAT(v)) { *out = X_MORPHO_GETFLOATVALUE(v); return true; }
//     return false;
// }


// DYNAMIFY_TYPE(double) fabs(DYNAMIFY_TYPE(double) x) {
//     DYNAMIFY_TYPE(double) y = x;
//     if (y < 0) y = -y;
//     return y;
// }


/** @brief Compares two values
 * @param a value to compare
 * @param b value to compare
 * @returns 0 if a and b are equal, a positive number if b\>a and a negative number if a\<b
 * @warning Requires that both values have the same type */
DYNAMIFY_TYPE(int) x_morpho_comparevalue(
    DYNAMIFY_TYPE(value) a,
    DYNAMIFY_TYPE(value) b,
    const pe_t_note types
) {
    //if (!morpho_ofsametype(a, b)) return MORPHO_NOTEQUAL;

    if (types == pe_t_note::FLOAT) {
        DYNAMIFY_TYPE(double) aa = X_MORPHO_GETFLOATVALUE(a);
        DYNAMIFY_TYPE(double) bb = X_MORPHO_GETFLOATVALUE(b);
        // Before there used to be a line here like:
        // if (x_morpho_doubleeqtest(aa, bb)) return MORPHO_EQUAL; // TODO: this kills us
        // return (bb>aa ? MORPHO_BIGGER : MORPHO_SMALLER);
        // but if's and ternary's on dyn_var's require buildit to do reevaluations
        // plus I'd want the C-compiler to figure out inlining on something like this
        return runtime::compare_double(aa, bb);
    }

    if (types == pe_t_note::NIL) {
        return MORPHO_EQUAL;
    }

    if (types == pe_t_note::INT) {
        DYNAMIFY_TYPE(int) aa = X_MORPHO_GETINTEGERVALUE(a);
        DYNAMIFY_TYPE(int) bb = X_MORPHO_GETINTEGERVALUE(b);
        return runtime::compare_int(aa, bb);
    }

    if (types == pe_t_note::BOOL) {
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
DYNAMIFY_TYPE(int) x_morpho_extendedcomparevalue(
    DYNAMIFY_TYPE(value) a,
    DYNAMIFY_TYPE(value) b,
    const pe_t_note a_t,
    const pe_t_note b_t
) {
    std::cerr << "received types of: " << a_t << " & " << b_t << "\n";
    if (a_t == pe_t_note::UNKNOWN || b_t == pe_t_note::UNKNOWN) return runtime::morpho_extendedcomparevalue(a, b);
    if (a_t == b_t) return x_morpho_comparevalue(a, b, a_t);

    DYNAMIFY_TYPE(value) aa=a, bb=b;

    if (a_t == pe_t_note::INT && b_t == pe_t_note::FLOAT) {
        aa = X_MORPHO_INTEGERTOFLOAT(a);
        return x_morpho_comparevalue(aa, bb, pe_t_note::FLOAT);
    } else if (a_t == pe_t_note::FLOAT && b_t == pe_t_note::INT) {
        bb = X_MORPHO_INTEGERTOFLOAT(b);
        return x_morpho_comparevalue(aa, bb, pe_t_note::FLOAT);
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