#ifndef SRC_BUILTIN_H
#define SRC_BUILTIN_H

#include <ostream>
#include "value.h" // <- the local one

namespace runtime {
	// If the function name exposed to buildit will be the same as the one appearing in transpiled code,
	// you can use this macro
	#define DECL_RUNTIME_FN(signature, fn_name) extern DYNAMIFY_TYPE(signature) fn_name;

	DECL_RUNTIME_FN(void(float), printfloat);
	DECL_RUNTIME_FN(void(int), printint);
	DECL_RUNTIME_FN(void(bool), printbool);
	DECL_RUNTIME_FN(void(void), printnil);
	DECL_RUNTIME_FN(void(void), printunimplemented);
	DECL_RUNTIME_FN(void(char*), printerr);
	DECL_RUNTIME_FN(void(void *, value), object_print);

	DECL_RUNTIME_FN(double(double, double), pow);
	DECL_RUNTIME_FN(double(double), round);
    DECL_RUNTIME_FN(double(double), fabs);

	DECL_RUNTIME_FN(value(value, value), add);

	#undef DECL_RUNTIME_FN
}

void print_wrapper_code(std::ostream &oss);

#endif // BUILTIN_H