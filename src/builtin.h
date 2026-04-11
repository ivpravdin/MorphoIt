#ifndef SRC_BUILTIN_H
#define SRC_BUILTIN_H

#include <ostream>
#include "value.h" // <- the local one

namespace runtime {
	// If the function name exposed to buildit will be the same as the one appearing in transpiled code,
	// you can use this macro
	#define DECL_RUNTIME_FN(signature, fn_name) extern DYNAMIFY_TYPE(signature) fn_name;

	DECL_RUNTIME_FN(void(char *), printerr);
	DECL_RUNTIME_FN(void(value), print);

	DECL_RUNTIME_FN(double(double, double), pow);
	DECL_RUNTIME_FN(double(double), round);
    DECL_RUNTIME_FN(double(double), fabs);

    DECL_RUNTIME_FN(int(value, value), morpho_extendedcomparevalue);

	DECL_RUNTIME_FN(value(value, value), op_add);
	DECL_RUNTIME_FN(value(value, value), op_sub);
	DECL_RUNTIME_FN(value(value, value), op_mul);
	DECL_RUNTIME_FN(value(value, value), op_div);
	DECL_RUNTIME_FN(value(value, value), op_pow);
	DECL_RUNTIME_FN(value(value), op_not);
	DECL_RUNTIME_FN(value(value, int, value*), call);
	DECL_RUNTIME_FN(value(const value *const, int, int, int, value *), call_userfn);

	#undef DECL_RUNTIME_FN
}

void print_wrapper_code(std::ostream &oss);

#endif // BUILTIN_H