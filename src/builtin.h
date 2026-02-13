#ifndef SRC_BUILTIN_H
#define SRC_BUILTIN_H

namespace runtime {
	dyn_var<void(uint64_t)> printint(builder::as_global("printint"));
	dyn_var<void(double)> printfloat(builder::as_global("printfloat"));
	dyn_var<void(double)> pow(builder::as_global("pow"));
}

static void print_wrapper_code(std::ostream &oss) {
	oss << "#include <stdio.h>\n";
	oss << "#include <stdlib.h>\n";
    oss << "#include <stdbool.h>\n";
	oss << "#include <math.h>\n";
	oss << "void printint(int x) {printf(\"%d\\n\", x);}\n";
    oss << "void printfloat(double x) {printf(\"%f\\n\", x);}\n";
}

#endif // BUILTIN_H