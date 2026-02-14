#ifndef SRC_BUILTIN_H
#define SRC_BUILTIN_H

#include <value.h> // <- the libmorpho one
#include "value.h" // <- the local one
namespace runtime {
	// If the function name exposed to buildit will be the same as the one appearing in transpiled code,
	// you can use this macro
	#define RUNTIME_FN(signature, fn_name) DYNAMIFY_TYPE(signature) fn_name(builder::as_global(#fn_name))

	RUNTIME_FN(void(float), printfloat);
	RUNTIME_FN(void(int), printint);
	RUNTIME_FN(void(bool), printbool);
	RUNTIME_FN(void(void), printnil);
	RUNTIME_FN(void(void), printunimplemented);
	RUNTIME_FN(void(char*), printstr);

	RUNTIME_FN(double(double, double), pow);

	#undef RUNTIME_FN
}

static void print_wrapper_code(std::ostream &oss) {
	oss << "#include <stdio.h>\n";
	oss << "#include <stdlib.h>\n";
    oss << "#include <stdbool.h>\n";
	oss << "#include <math.h>\n";

	oss << "void printint(int x) {printf(\"%d\\n\", x);}\n";
    oss << "void printfloat(double x) {printf(\"%g\\n\", x);}\n";
	oss << "void printbool(bool x) {printf(\"%s\\n\", x ? \"" << MORPHO_TRUESTRING << "\" : \"" << MORPHO_FALSESTRING << "\");}\n";
    oss << "void printstr(char x[]) {printf(\"%s\\n\", x);}\n";
	oss << "void printnil() {printf(\"" << MORPHO_NILSTRING << "\\n\");}\n";
	oss << "void printunimplemented() {printf(\"(print has not been implemented for this type)\\n\");}\n";
}

#endif // BUILTIN_H