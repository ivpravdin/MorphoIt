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

	RUNTIME_FN(double(double, double), pow);

	RUNTIME_FN(double(double), round);
    RUNTIME_FN(double(double), fabs);

	RUNTIME_FN(bool(double, double), doubleeqtest);
	RUNTIME_FN(int(double, double), comparedoubles);
	RUNTIME_FN(int(int, int), compareints);
	RUNTIME_FN(int(bool, bool), comparebools);
	// RUNTIME_FN(int(int, int), compareobjects); // This is not implemented yet, and will require a way to compare objects

	#undef RUNTIME_FN
}

void print_wrapper_code(std::ostream &oss) {
	oss << "#include <stdio.h>\n";
	oss << "#include <stdlib.h>\n";
    oss << "#include <stdbool.h>\n";
	oss << "#include <math.h>\n";

	oss << "#define DBL_EPSILON 0.000001\n";

	oss << "void printint(int x) {printf(\"%d\\n\", x);}\n";
    oss << "void printfloat(double x) {printf(\"%g\\n\", x);}\n";
	oss << "void printbool(bool x) {printf(\"%s\\n\", x ? \"" << MORPHO_TRUESTRING << "\" : \"" << MORPHO_FALSESTRING << "\");}\n";
    oss << "void printerr(char x[]) {fprintf(stderr, \"%s\\n\", x);}\n";
	oss << "void printnil() {printf(\"" << MORPHO_NILSTRING << "\\n\");}\n";

	oss << "bool doubleeqtest(double a, double b) {\n";
	oss << "    return fabs(a - b) <= DBL_EPSILON;\n";
	oss << "}\n";

	oss << "int comparedoubles(double a, double b) {\n";
	oss << "    if (doubleeqtest(a, b)) return " << MORPHO_EQUAL << ";\n";
	oss << "    return (b > a ? " << MORPHO_BIGGER << " : " << MORPHO_SMALLER << ");\n";
	oss << "}\n";

	oss << "int compareints(int a, int b) {\n";
	oss << "    if (a == b) return " << MORPHO_EQUAL << ";\n";
	oss << "    return (b > a ? " << MORPHO_BIGGER << " : " << MORPHO_SMALLER << ");\n";
	oss << "}\n";

	oss << "int comparebools(bool a, bool b) {\n";
	oss << "    return (a != b);\n";
	oss << "}\n";
}