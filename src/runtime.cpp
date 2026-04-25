#include "runtime.h"

#include <value.h>

#include "pe_vm_consts.h"
#include "pe_vm.h"
#include "generated_header.h"

namespace runtime {
	// If the function name exposed to buildit will be the same as the one appearing in transpiled code,
	// you can use this macro
	#define RUNTIME_FN(signature, fn_name) DYNAMIFY_TYPE(signature) fn_name(builder::with_name(#fn_name))

	RUNTIME_FN(void(char *), printerr);
	RUNTIME_FN(void(value), print);

	RUNTIME_FN(double(double, double), pow);
	RUNTIME_FN(double(double), round);
    RUNTIME_FN(double(double), fabs);

	RUNTIME_FN(int(value, value), morpho_extendedcomparevalue);
    RUNTIME_FN(int(double, double), compare_double);
    RUNTIME_FN(int(double, double), compare_int);

	RUNTIME_FN(value(value, value), op_add);
	RUNTIME_FN(value(value, value), op_sub);
	RUNTIME_FN(value(value, value), op_mul);
	RUNTIME_FN(value(value, value), op_div);
	RUNTIME_FN(value(value, value), op_pow);
	RUNTIME_FN(value(value), op_not);
	RUNTIME_FN(value(value, int, value*), call);

	#undef RUNTIME_FN
}

std::string get_mangled_fn_name(const userfn_sig &sig, bool genericize) {
    std::string name = std::string(USERFN_NAME_PREFIX);
    if (MORPHO_ISOBJECT(sig.objfn->name)) {
        name += MORPHO_GETCSTRING(sig.objfn->name);
        name += "_";
    }
    name += std::to_string((uintptr_t) sig.objfn);

	// use genericize to force to get generic name, even if is a specialized fn
	if (!genericize && sig.argtypes.has_value()) {
		for (pe_t_note t : sig.argtypes.value()) {
			name += "_t" + std::to_string(t);
		}
	}
    return std::move(name);
}

std::string get_mangled_fnobj_name(const userfn_sig &sig, bool genericize) {
    return get_mangled_fn_name(sig, genericize) + USERFNOBJ_NAME_SUFFIX;
}

std::string generate_fnobj_definition(const userfn_sig &sig, bool genericize) {
	return "const struct userfn_object "
		+ get_mangled_fnobj_name(sig, genericize)
		+ " = { "
		+ ".type = " + std::to_string(objectfunctiontype) + ", " +
		+ ".fn = " + get_mangled_fn_name(sig, genericize)
		+ " };\n";
}

void print_wrapper_code(std::ostream &oss) {
	oss << "#define FOR_RUNTIME\n";
	oss << header;
}