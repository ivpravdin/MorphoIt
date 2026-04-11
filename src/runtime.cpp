#include "runtime.h"

#include <value.h>

#include "pe_vm_consts.h"

namespace runtime {
	// If the function name exposed to buildit will be the same as the one appearing in transpiled code,
	// you can use this macro
	#define RUNTIME_FN(signature, fn_name) DYNAMIFY_TYPE(signature) fn_name(builder::as_global(#fn_name))

	RUNTIME_FN(void(char *), printerr);
	RUNTIME_FN(void(value), print);

	RUNTIME_FN(double(double, double), pow);
	RUNTIME_FN(double(double), round);
    RUNTIME_FN(double(double), fabs);

	RUNTIME_FN(int(value, value), morpho_extendedcomparevalue);

	RUNTIME_FN(value(value, value), op_add);
	RUNTIME_FN(value(value, value), op_sub);
	RUNTIME_FN(value(value, value), op_mul);
	RUNTIME_FN(value(value, value), op_div);
	RUNTIME_FN(value(value, value), op_pow);
	RUNTIME_FN(value(value), op_not);
	RUNTIME_FN(value(value, int, value*), call);

	#undef RUNTIME_FN
}

constexpr char header[] = {
	#embed "pe_vm_consts.h"
	, '\n',
	#embed "runtime_header.c"
	, '\0'
};
void print_wrapper_code(std::ostream &oss) {
	oss << "#define RUNTIME_HEADER_C\n";
	oss << header;
}