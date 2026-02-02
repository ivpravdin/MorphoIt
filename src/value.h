#ifndef SRC_VALUE_H
#define SRC_VALUE_H

#define DYNAMIFY_TYPE(type) dyn_var<type>

#define CAST_DYN_VAR(type, val) (bitcast<type>(val))

static inline DYNAMIFY_TYPE(value) x_doubletovalue(dyn_var<double> num) {
  return CAST_DYN_VAR(value, num);
}

static inline DYNAMIFY_TYPE(double) x_valuetodouble(dyn_var<value> num) {
  return CAST_DYN_VAR(double, num);
}

#define X_MORPHO_INTEGER(x)         (((DYNAMIFY_TYPE(uint64_t)(x)) & LOWER_WORD) | QNAN | TAG_INT)
#define X_MORPHO_GETINTEGERVALUE(v)   ((DYNAMIFY_TYPE(int))((DYNAMIFY_TYPE(uint32_t))(v & LOWER_WORD)))
#define X_MORPHO_FLOAT(x)             x_doubletovalue(x)
#define X_MORPHO_GETFLOATVALUE(v)     x_valuetodouble(v)


#endif // SRC_VALUE_H