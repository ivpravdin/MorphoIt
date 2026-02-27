#include "builtin.h"

#include <value.h>

namespace runtime {
	// If the function name exposed to buildit will be the same as the one appearing in transpiled code,
	// you can use this macro
	#define RUNTIME_FN(signature, fn_name) DYNAMIFY_TYPE(signature) fn_name(builder::as_global(#fn_name))

	RUNTIME_FN(void(float), printfloat);
	RUNTIME_FN(void(int), printint);
	RUNTIME_FN(void(bool), printbool);
	RUNTIME_FN(void(void), printnil);
	RUNTIME_FN(void(void), printunimplemented);
	RUNTIME_FN(void(char*), printerr);
	RUNTIME_FN(void(void *, value), object_print);

	RUNTIME_FN(double(double, double), pow);

	RUNTIME_FN(double(double), round);
    RUNTIME_FN(double(double), fabs);

	#undef RUNTIME_FN
}

void print_wrapper_code(std::ostream &oss) {
	oss << "#include <stdio.h>\n";
	oss << "#include <stdlib.h>\n";
    oss << "#include <stdbool.h>\n";
	oss << "#include <math.h>\n";
	oss << "#include <morpho/morpho.h>\n";
	oss << "#include <morpho/object.h>\n";

	oss << "void printint(int x) {printf(\"%d\\n\", x);}\n";
    oss << "void printfloat(double x) {printf(\"%g\\n\", x);}\n";
	oss << "void printbool(bool x) {printf(\"%s\\n\", x ? \"" << MORPHO_TRUESTRING << "\" : \"" << MORPHO_FALSESTRING << "\");}\n";
    oss << "void printerr(char x[]) {fprintf(stderr, \"%s\\n\", x);}\n";
	oss << "void printnil() {printf(\"" << MORPHO_NILSTRING << "\\n\");}\n";
	oss << "void printstring(char x[]) {printf(\"%s\\n\", x);}\n";
}