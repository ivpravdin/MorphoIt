#include "pe_vm.h"

#include <exception>
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include "builder/lib/utils.h"

#include "value.h"
#include "runtime.h"
#include "pe_vm_consts.h"
#include "userfn.h"


// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;
using std::vector;

dyn_var<value> op_add(dyn_var<value> lhs, dyn_var<value> rhs, pe_t_note lh_t, pe_t_note rh_t) {
    if (lh_t == pe_t_note::INT) {
        if (rh_t == pe_t_note::INT) {
            return X_MORPHO_INTEGER(X_MORPHO_GETINTEGERVALUE(lhs) + X_MORPHO_GETINTEGERVALUE(rhs));
        } else if (rh_t == pe_t_note::FLOAT) {
            return X_MORPHO_FLOAT(X_MORPHO_GETINTEGERVALUE(lhs) + X_MORPHO_GETFLOATVALUE(rhs));
        }
    } else if (lh_t == pe_t_note::FLOAT) {
        if (rh_t == pe_t_note::INT) {
            return X_MORPHO_FLOAT(X_MORPHO_GETFLOATVALUE(lhs) + X_MORPHO_GETINTEGERVALUE(rhs));
        } else if (rh_t == pe_t_note::FLOAT) {
            return X_MORPHO_FLOAT(X_MORPHO_GETFLOATVALUE(lhs) + X_MORPHO_GETFLOATVALUE(rhs));
        }
    }

    return runtime::op_add(lhs, rhs);
}

dyn_var<value> op_sub(dyn_var<value> lhs, dyn_var<value> rhs, pe_t_note lh_t, pe_t_note rh_t) {
    if (lh_t == pe_t_note::INT) {
        if (rh_t == pe_t_note::INT) {
            return X_MORPHO_INTEGER(X_MORPHO_GETINTEGERVALUE(lhs) - X_MORPHO_GETINTEGERVALUE(rhs));
        } else if (rh_t == pe_t_note::FLOAT) {
            return X_MORPHO_FLOAT(X_MORPHO_GETINTEGERVALUE(lhs) - X_MORPHO_GETFLOATVALUE(rhs));
        }
    } else if (lh_t == pe_t_note::FLOAT) {
        if (rh_t == pe_t_note::INT) {
            return X_MORPHO_FLOAT(X_MORPHO_GETFLOATVALUE(lhs) - X_MORPHO_GETINTEGERVALUE(rhs));
        } else if (rh_t == pe_t_note::FLOAT) {
            return X_MORPHO_FLOAT(X_MORPHO_GETFLOATVALUE(lhs) - X_MORPHO_GETFLOATVALUE(rhs));
        }
    }

    return runtime::op_sub(lhs, rhs);
}

dyn_var<value> op_mul(dyn_var<value> lhs, dyn_var<value> rhs, pe_t_note lh_t, pe_t_note rh_t) {
    if (lh_t == pe_t_note::INT) {
        if (rh_t == pe_t_note::INT) {
            return X_MORPHO_INTEGER(X_MORPHO_GETINTEGERVALUE(lhs) * X_MORPHO_GETINTEGERVALUE(rhs));
        } else if (rh_t == pe_t_note::FLOAT) {
            return X_MORPHO_FLOAT(X_MORPHO_GETINTEGERVALUE(lhs) * X_MORPHO_GETFLOATVALUE(rhs));
        }
    } else if (lh_t == pe_t_note::FLOAT) {
        if (rh_t == pe_t_note::INT) {
            return X_MORPHO_FLOAT(X_MORPHO_GETFLOATVALUE(lhs) * X_MORPHO_GETINTEGERVALUE(rhs));
        } else if (rh_t == pe_t_note::FLOAT) {
            return X_MORPHO_FLOAT(X_MORPHO_GETFLOATVALUE(lhs) * X_MORPHO_GETFLOATVALUE(rhs));
        }
    }

    return runtime::op_add(lhs, rhs);
}

dyn_var<value> op_div(dyn_var<value> lhs, dyn_var<value> rhs, pe_t_note lh_t, pe_t_note rh_t) {
    if (lh_t == pe_t_note::INT) {
        if (rh_t == pe_t_note::INT) {
            return X_MORPHO_FLOAT((dyn_var<double>) X_MORPHO_GETINTEGERVALUE(lhs) / (dyn_var<double>) X_MORPHO_GETINTEGERVALUE(rhs));
        } else if (rh_t == pe_t_note::FLOAT) {
            return X_MORPHO_FLOAT(X_MORPHO_GETINTEGERVALUE(lhs) / X_MORPHO_GETFLOATVALUE(rhs));
        }
    } else if (lh_t == pe_t_note::FLOAT) {
        if (rh_t == pe_t_note::INT) {
            return X_MORPHO_FLOAT(X_MORPHO_GETFLOATVALUE(lhs) / X_MORPHO_GETINTEGERVALUE(rhs));
        } else if (rh_t == pe_t_note::FLOAT) {
            return X_MORPHO_FLOAT(X_MORPHO_GETFLOATVALUE(lhs) / X_MORPHO_GETFLOATVALUE(rhs));
        }
    }

    return runtime::op_div(lhs, rhs);
}

dyn_var<value> op_pow(dyn_var<value> lhs, dyn_var<value> rhs, pe_t_note lh_t, pe_t_note rh_t) {
    if (lh_t == pe_t_note::INT) {
        if (rh_t == pe_t_note::INT) {
            return X_MORPHO_FLOAT(runtime::pow((dyn_var<double>) X_MORPHO_GETINTEGERVALUE(lhs), (dyn_var<double>) X_MORPHO_GETINTEGERVALUE(rhs)));
        } else if (rh_t == pe_t_note::FLOAT) {
            return X_MORPHO_FLOAT(runtime::pow((dyn_var<double>) X_MORPHO_GETINTEGERVALUE(lhs), X_MORPHO_GETFLOATVALUE(rhs)));
        }
    } else if (lh_t == pe_t_note::FLOAT) {
        if (rh_t == pe_t_note::INT) {
            return X_MORPHO_FLOAT(runtime::pow(X_MORPHO_GETFLOATVALUE(lhs), (dyn_var<double>) X_MORPHO_GETINTEGERVALUE(rhs)));
        } else if (rh_t == pe_t_note::FLOAT) {
            return X_MORPHO_FLOAT(runtime::pow(X_MORPHO_GETFLOATVALUE(lhs), X_MORPHO_GETFLOATVALUE(rhs)));
        }
    }

    return runtime::op_pow(lhs, rhs);
}

dyn_var<value> op_not(dyn_var<value> val, pe_t_note t) {
    if (t == pe_t_note::UNKNOWN) return runtime::op_not(val);

    if (t == pe_t_note::BOOL) {
        return MORPHO_BOOL(!MORPHO_GETBOOLVALUE(val));
    }

    return MORPHO_FALSE;
}

// struct static_reg_t {
//     static_var<value> val;
//     static_var<pe_t_note> t;
// };



dyn_var<value> morpho_vm(
    dyn_var<value *> args, // all buildit examples have dyns first, idk why
    const int ninstructions,
    const instruction * const instructions,
    const userfn_sig &thisfn_sig,
    userfn_details &thisfn_deets,
    userfn_map &subfn_asts,
    const bool is_main
) {
    const objectfunction *const thisfn = thisfn_sig.objfn;
    // I'm not super sure about why, but there is something a little stinky
    // about the order in which you initalize static and dyn vars
    // because rearranging this can lead to errors where variables are "missing"
    // in generated code. Just be warned

    // static var init'ing
    std::vector<static_var<pe_t_note>> global_type(PE_NUM_GLOBALS);
    for (static_var<size_t> i = 0; i < PE_NUM_GLOBALS; i++) {
        global_type[i] = pe_t_note::UNKNOWN;
    }
    std::vector<static_var<value>> reg_stat(thisfn->nregs);
    std::vector<static_var<pe_t_note>> reg_type(thisfn->nregs);
    for (static_var<size_t> i = 0; i < thisfn->nregs; i++) {
        reg_stat[i] = MORPHO_NIL;
        reg_type[i] = pe_t_note::UNKNOWN;
    }
    reg_stat[0] = MORPHO_OBJECT(thisfn);
    reg_type[0] = pe_t_note::OBJECT;
    // args start from reg 1 and on
    if (thisfn_sig.argtypes.has_value()) {
        size_t nargtypes = thisfn_sig.argtypes->size();
        for (static_var<size_t> i = 0; i < nargtypes; i++) {
            reg_type[i + 1] = thisfn_sig.argtypes.value()[i];
        }
    }

    // dynvar init'ing
    dyn_var<value[PE_NUM_GLOBALS]> globals = builder::with_name(PE_GLOBALSBUF_NAME);
    // main not only can't call itself recursively, it also doesn't follow the
    // mangled naming convention, so this wouldn't work.
    // nonetheless maybe this ought to be made to work
    dyn_var<value[]> reg = builder::with_name("reg", true);
    builder::resize_arr(reg, thisfn->nregs);
    const size_t total_args = thisfn->nargs + thisfn->nopt;
    for (static_var<size_t> i = 0; i < total_args; i++) {
        reg[i + 1] = args[i + 1];
    }

    // I'm not sure why, but if you move this too high up, things start to break
    if (!is_main) {
        // can't specialize until calltime, so even if this function has a specialized
        // signature we force it to use the generic name
        dyn_var<const struct literal_userfn_object> runtime_fn_obj = builder::with_name(
            thisfn_deets.get_runtime_fnobj_name(true)
        );
        reg[0] = X_MORPHO_OBJECT(&runtime_fn_obj);
    } else {
        dyn_var<literal_userfn> morpho_initialize = builder::with_name("morpho_initialize");
        morpho_initialize();
    }


    dyn_var<value> left = builder::with_name("left", true), right = builder::with_name("right", true);
    static_var<instruction> pc = thisfn->entry;

    while (pc < ninstructions) {
        const instruction bc = instructions[pc];
        const uint8_t a = DECODE_A(bc), b = DECODE_B(bc), c = DECODE_C(bc);
        const uint16_t bx = DECODE_Bx(bc);
        const int16_t sbx = DECODE_sBx(bc);
        switch (DECODE_OP(bc)) {
            case OP_NOP:
                break;

            case OP_MOV:
                reg[a] = reg[b];
                reg_stat[a] = reg_stat[b];
                reg_type[a] = reg_type[b];
                break;

            case OP_LCT:
                if (MORPHO_ISFUNCTION(thisfn->konst.data[bx])) {
                    const objectfunction *const morpho_fn = MORPHO_GETFUNCTION(thisfn->konst.data[bx]);

                    // this is less solid than I thought: name can be the emptystring, so there could be naming conflicts here
                    dyn_var<const struct literal_userfn_object> runtime_fn_obj = builder::with_name(
                            userfn_sig { .objfn = morpho_fn }.get_mangled_fnobj_name(true)
                    );
                    reg[a] = X_MORPHO_OBJECT(&runtime_fn_obj);
                    reg_stat[a] = thisfn->konst.data[bx];
                } else {
                    // for other objects...
                    // maybe generate C code equivalent to "get it from the RUNTIME
                    // constant table!" I think that's perfect
                    // aside from any objects types want to statically eval
                    reg[a] = thisfn->konst.data[bx];
                    reg_stat[a] = MORPHO_NIL;
                }
                reg_type[a] = gettypeannotation(thisfn->konst.data[bx]);
                // std::cerr << "Loaded const of type: " << reg_type[a] << "\ninstructions";
                break;

            case OP_ADD:
                left = reg[b], right = reg[c];
                reg[a] = op_add(left, right, reg_type[b], reg_type[c]);
                reg_stat[a] = MORPHO_NIL;
                reg_type[a] = arith_binop_typerule(reg_type[b], reg_type[c]);
                break;

            case OP_SUB:
                left = reg[b], right = reg[c];

                reg[a] = op_sub(left, right, reg_type[b], reg_type[c]);
                reg_stat[a] = MORPHO_NIL;
                reg_type[a] = arith_binop_typerule(reg_type[b], reg_type[c]);
                break;

            case OP_MUL:
                left = reg[b], right = reg[c];
                reg[a] = op_mul(left, right, reg_type[b], reg_type[c]);
                reg_stat[a] = MORPHO_NIL;
                reg_type[a] = arith_binop_typerule(reg_type[b], reg_type[c]);

                break;

            case OP_DIV:
                left = reg[b], right = reg[c];
                reg[a] = op_div(left, right, reg_type[b], reg_type[c]);
                reg_stat[a] = MORPHO_NIL;
                reg_type[a] = pe_t_note::FLOAT;
                break;

            case OP_POW: //POW
                left = reg[b];
                right = reg[c];
                reg[a] = op_pow(left, right, reg_type[b], reg_type[c]);
                reg_stat[a] = MORPHO_NIL;
                reg_type[a] = pe_t_note::FLOAT;
                break;

            case OP_EQ:
                left = reg[b];
                right = reg[c];

                // reg[a] = X_MORPHO_BOOL(!x_morpho_extendedcomparevalue(left, right, reg_type[b], reg_type[c]));
                reg[a] = X_MORPHO_BOOL(!x_morpho_extendedcomparevalue(left, right, reg_type[b], reg_type[c]));
                reg_stat[a] = MORPHO_NIL;
                reg_type[a] = pe_t_note::BOOL;
                break;
            case OP_NEQ:
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(x_morpho_extendedcomparevalue(left, right, reg_type[b], reg_type[c]));
                reg_stat[a] = MORPHO_NIL;
                reg_type[a] = pe_t_note::BOOL;
                break;
            case OP_NOT:
                left = reg[b];

                reg[a] = op_not(left, reg_type[b]);
                reg_stat[a] = MORPHO_NIL;
                reg_type[a] = pe_t_note::BOOL;
                break;
            case OP_LT: //LT
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(x_morpho_extendedcomparevalue(left, right, reg_type[b], reg_type[c]) > MORPHO_EQUAL);
                reg_stat[a] = MORPHO_NIL;
                reg_type[a] = pe_t_note::BOOL;
                break;
            case OP_LE: //LT
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(x_morpho_extendedcomparevalue(left, right, reg_type[b], reg_type[c]) >= MORPHO_EQUAL);
                reg_stat[a] = MORPHO_NIL;
                reg_type[a] = pe_t_note::BOOL;
                break;
            case OP_B: // B
                pc+=sbx;
                break;
            case OP_BIF: // BIF
                left=reg[a];

                if (X_MORPHO_ISTRUE(left)) pc+=sbx;
                break;
            case OP_BIFF: // BIFF
                left=reg[a];

                if (X_MORPHO_ISFALSE(left)) pc+=sbx;
                break;

            case OP_CALL: // CALL (no support for optional arguments yet)
                left = reg[a];
                if (MORPHO_ISFUNCTION(reg_stat[a])) {
                    objectfunction *fn = MORPHO_GETFUNCTION(reg_stat[a]);

                    const size_t nargs =  fn->nargs + fn->nopt;
                    vector<pe_t_note>argtypes(nargs);
                    static_var<bool> nottrivial = false; // trivial meaning no types are known, so equivalent to generic
                    for (static_var<size_t> i = 0; i < nargs; i++) {
                        // args start from reg[a+1]
                        argtypes[i] = reg_type[a + 1 + i];
                        if (reg_type[a + 1 + i] != pe_t_note::UNKNOWN)
                            nottrivial = true;
                    }

                    // generate fn signature
                    userfn_sig sig = { 
                        .objfn = fn,
                        .argtypes = nottrivial
                            ? std::make_optional(std::move(argtypes)) 
                            : std::nullopt
                    };

                    // send back signature to PE a specialized version of the fn
                    if (subfn_asts.count(sig) == 0) {
                        throw sig;
                    }

                    // now specialized, we can get type, etc
                    reg_type[a] = subfn_asts.at(sig).get_returntype();
                    dyn_var<literal_userfn *> fn_ptr_literal = builder::with_name(sig.get_mangled_fn_name(false));
                    fprintf(stderr, "calling fn %s gave returntype: %s\n", sig.get_mangled_fn_name(false).c_str(), gettypename(reg_type[a]).c_str());

                    reg[a] = fn_ptr_literal(&reg[a]);
                } else {
                    reg[a]=runtime::call(left, b, &reg[a]);
                    reg_type[a] = pe_t_note::UNKNOWN;
                }

                // global state is opaque after a function call
                for (static_var<size_t> i = 0; i < PE_NUM_GLOBALS; i++) {
                    global_type[i] = pe_t_note::UNKNOWN;
                }
                // for now function calls will be completely opaque
                reg_stat[a] = MORPHO_NIL;
                break;
            case OP_RETURN:
                if (a>0) {
                    fprintf(stderr, "Function %s given returntype: %s\n", thisfn_sig.get_mangled_fn_name(false).c_str(), gettypename(reg_type[b]).c_str());
                    thisfn_deets.update_returntype(reg_type[b]);
                    return reg[b];
                }
                else {
                    thisfn_deets.update_returntype(pe_t_note::NIL);
                    fprintf(stderr, "Function %s given returntype: NIL (because it has no ret val)\n", thisfn_sig.get_mangled_fn_name(false).c_str());
                    return MORPHO_NIL;
                }
                break;
            case OP_LGL: // LGL
                reg[a]      = globals[bx];
                reg_stat[a] = MORPHO_NIL;
                reg_type[a] = global_type[bx];
                break;
            case OP_SGL: // SGL
                globals[bx]     = reg[a];
                global_type[bx] = reg_type[a];
                break;
            case OP_PRINT: // PRINT
                left=reg[a];
                runtime::print(left);
                break;
            case OP_END: // END
                return EXIT_SUCCESS;
            default:
                throw UnimplementedInstructionException();
        }
        pc++;
    }
    throw OutOfBoundsPCException();
}

// dyn_var<value> morpho_vm(
//     dyn_var<value *> args,
//     const int ninstructions,
//     const instruction *const instructions,
//     const objectfunction *const thisfn,
//     const bool is_main,
//     std::map<uintptr_t, block::stmt::Ptr> &subfn_asts
// ) {
//     // std::cerr << "toplevel\ninstructions";
//     // declaration of globals is handled in header.c, no other way to make it
//     // actually global, apparently
//     return morpho_vm_rec(
//         args,
//         ninstructions,
//         instructions,
//         thisfn,
//         subfn_asts
//     );
// }