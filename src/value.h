#ifndef SRC_VALUE_H
#define SRC_VALUE_H

#include "builder/static_var.h"
#include "builder/dyn_var.h"
#include "morpho_header.h"

#define DYNAMIFY_TYPE(type) builder::dyn_var<type>

// static cast
#define S_CAST_DYN_VAR(type, exp) (static_cast<DYNAMIFY_TYPE(type)>(exp))
// reinterpret cast
#define RI_CAST_DYN_VAR(type, val) (builder::bitcast<type>(val))

static inline DYNAMIFY_TYPE(value) x_doubletovalue(DYNAMIFY_TYPE(double) num) {
  return RI_CAST_DYN_VAR(value, num);
}

static inline DYNAMIFY_TYPE(double) x_valuetodouble(DYNAMIFY_TYPE(value) num) {
  return RI_CAST_DYN_VAR(double, num);
}

#define X_MORPHO_OFSAMETYPE(a, b) (MORPHO_GETTYPE(a) == MORPHO_GETTYPE(b))
#define X_MORPHO_ISFALSE(a) (MORPHO_ISNIL(a) || (MORPHO_ISBOOL(a) && (MORPHO_GETBOOLVALUE(a)==false)))
#define X_MORPHO_ISTRUE(a) (!X_MORPHO_ISFALSE((a)))

#define X_MORPHO_INTEGER(x)         (((DYNAMIFY_TYPE(uint64_t)(x)) & LOWER_WORD) | QNAN | TAG_INT)
#define X_MORPHO_GETINTEGERVALUE(v)   ((DYNAMIFY_TYPE(int))((DYNAMIFY_TYPE(uint32_t))(v & LOWER_WORD)))
#define X_MORPHO_FLOAT(x)             x_doubletovalue(x)
#define X_MORPHO_GETFLOATVALUE(v)     x_valuetodouble(v)
#define X_MORPHO_BOOL(x)         (((DYNAMIFY_TYPE(uint64_t)(x)) & LOWER_WORD) | QNAN | TAG_BOOL)
#define X_MORPHO_GETBOOLVALUE(v)   ((DYNAMIFY_TYPE(bool))((DYNAMIFY_TYPE(uint32_t))(v & LOWER_WORD)))
#define X_MORPHO_OBJECT(x)            ((DYNAMIFY_TYPE(value)) (TAG_OBJ | QNAN | (DYNAMIFY_TYPE(uint64_t))(DYNAMIFY_TYPE(uintptr_t))(x)))
#define X_MORPHO_GETOBJECT(v)         ((DYNAMIFY_TYPE(dyn_object *)) (DYNAMIFY_TYPE(uintptr_t)) ((v) & ~(TAG_OBJ | QNAN)))
// #define X_MORPHO_GETOBJECTTYPE(val)           (X_MORPHO_GETOBJECT(val)->type)
// #define X_MORPHO_GETSTRINGVALUE(val)          (X_MORPHO_GETOBJECT(val)->hsh.str)

/** Conversion of integer to a float */
#define X_MORPHO_INTEGERTOFLOAT(x) (X_MORPHO_FLOAT(DYNAMIFY_TYPE(double) X_MORPHO_GETINTEGERVALUE((x))))

/** Conversion of a float to an integer with rounding */
#define X_MORPHO_FLOATTOINTEGER(x) (X_MORPHO_INTEGER((int) round(X_MORPHO_GETFLOATVALUE((x)))))

DYNAMIFY_TYPE(int) x_morpho_comparevalue(DYNAMIFY_TYPE(value) a, DYNAMIFY_TYPE(value) b);
DYNAMIFY_TYPE(int) x_morpho_extendedcomparevalue(DYNAMIFY_TYPE(value) a, DYNAMIFY_TYPE(value) b);

struct dyn_object {
    static constexpr const char* type_name = "object";
    
    DYNAMIFY_TYPE(objecttype) type = builder::with_name("type");
    DYNAMIFY_TYPE(int) status;        // enum becomes int
    DYNAMIFY_TYPE(hash) hsh;
    DYNAMIFY_TYPE(dyn_object*) next;
};

#endif // SRC_VALUE_H