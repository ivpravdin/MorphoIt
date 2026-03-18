#include "builtin.h"

#include <value.h>

namespace runtime {
	// If the function name exposed to buildit will be the same as the one appearing in transpiled code,
	// you can use this macro
	#define RUNTIME_FN(signature, fn_name) DYNAMIFY_TYPE(signature) fn_name(builder::as_global(#fn_name))

	RUNTIME_FN(void(char *), printerr);
	RUNTIME_FN(void(value), print);

	RUNTIME_FN(double(double, double), pow);

	RUNTIME_FN(double(double), round);
    RUNTIME_FN(double(double), fabs);

	RUNTIME_FN(value(value, value), add);
	RUNTIME_FN(value(value, int, value*), call);
	RUNTIME_FN(int(value, value), morpho_extendedcomparevalue);
	RUNTIME_FN(value(value, value), op_div);

	#undef RUNTIME_FN
}

constexpr char header[] = {
	#embed "header.c"
	, '\0'
};
void print_wrapper_code(std::ostream &oss) {
	oss << header;
}