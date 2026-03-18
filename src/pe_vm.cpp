#include "pe_vm.h"

#include "value.h"
#include "builtin.h"

#include "builder/dyn_var.h"
#include "builder/static_var.h"

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

constexpr size_t NUM_REGISTERS = 255;
constexpr size_t NUM_GLOBALS   = 100;

constexpr size_t MAX_INSTRUCTIONS   = 25;
// typedef enum {
//     ERR = 0,
//     NIL,
//     BOOL,
//     INT,
//     FLOAT,
//     CMPLX,
//     OBJECT,
//     DYN
// } pe_t;


static_var<pe_t> value2petype(static_var<value> v) {
    if (MORPHO_ISFLOAT(v)) {
        return PE_FLOAT_T;
    } else if (MORPHO_ISBOOL(v)) {
        return PE_BOOL_T;
    } else if (MORPHO_ISINTEGER(v)) {
        return PE_INT_T;
    } else if (MORPHO_ISNIL(v)) {
        return PE_NIL_T;
    } else if (MORPHO_ISOBJECT(v)) {
        return PE_OBJ_T;
    }

    return PE_ERR_T;
}

static_var<pe_t> integralBinOpTypeRule(static_var<pe_t> lht, static_var<pe_t> rht) {
    const bool lisIorD = lht & (PE_DYN_T | PE_INT_T | PE_FLOAT_T);
    const bool risIorD = rht & (PE_DYN_T | PE_INT_T | PE_FLOAT_T);

    if (!lisIorD || !risIorD) return PE_ERR_T;

    if (lht == PE_DYN_T || rht == PE_DYN_T) {
        return PE_DYN_T;
    }

    if (lht == PE_FLOAT_T || rht == PE_FLOAT_T) {
        return PE_FLOAT_T;
    } else {
        return PE_INT_T;
    }
}

dyn_var<value> op_add(dyn_var<value> left, dyn_var<value> right, static_var<pe_t> ltype, static_var<pe_t> rtype) {
    if (ltype == PE_FLOAT_T) {
        if (rtype == PE_FLOAT_T)
            return X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) + X_MORPHO_GETFLOATVALUE(right));
        else if (rtype == PE_INT_T)
            return X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) + X_MORPHO_GETINTEGERVALUE(right));
        // else // (right_type is DYN) ...
    } else if (ltype == PE_INT_T) {
        if (rtype == PE_FLOAT_T)
            return X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) + X_MORPHO_GETFLOATVALUE(right));
        else if (rtype == PE_INT_T)
            return X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) + X_MORPHO_GETINTEGERVALUE(right));
        // else // right_type is DYN
    }

    return runtime::add(left, right);
}

dyn_var<value> op_sub_dyn(dyn_var<value> left, dyn_var<value> right) {
    if (MORPHO_ISFLOAT(left)) {
        if (MORPHO_ISFLOAT(right)) {
            return X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) - X_MORPHO_GETFLOATVALUE(right));
        } else if (MORPHO_ISINTEGER(right)) {
            return X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) - X_MORPHO_GETINTEGERVALUE(right));
        }
    } else if (MORPHO_ISINTEGER(left)) {
        if (MORPHO_ISFLOAT(right)) {
            return X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) - X_MORPHO_GETFLOATVALUE(right));
        } else if (MORPHO_ISINTEGER(right)) {
            return X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) - X_MORPHO_GETINTEGERVALUE(right));
        }
    }

    // type error...
    return MORPHO_NIL;
}

dyn_var<value> op_sub(dyn_var<value> left, dyn_var<value> right, static_var<pe_t> ltype, static_var<pe_t> rtype) {
    if (ltype == PE_FLOAT_T) {
        if (rtype == PE_FLOAT_T)
            return X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) - X_MORPHO_GETFLOATVALUE(right));
        else if (rtype == PE_INT_T)
            return X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) - X_MORPHO_GETINTEGERVALUE(right));
        // else // (right_type is DYN) ...
    } else if (ltype == PE_INT_T) {
        if (rtype == PE_FLOAT_T)
            return X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) - X_MORPHO_GETFLOATVALUE(right));
        else if (rtype == PE_INT_T)
            return X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) - X_MORPHO_GETINTEGERVALUE(right));
        // else // right_type is DYN
    }

    return op_sub_dyn(left, right);
}


dyn_var<value> op_mul_dyn(dyn_var<value> left, dyn_var<value> right) {
    if (MORPHO_ISFLOAT(left)) {
        if (MORPHO_ISFLOAT(right)) {
            return X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) * X_MORPHO_GETFLOATVALUE(right));
        } else if (MORPHO_ISINTEGER(right)) {
            return X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) * X_MORPHO_GETINTEGERVALUE(right));
        }
    } else if (MORPHO_ISINTEGER(left)) {
        if (MORPHO_ISFLOAT(right)) {
            return X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) * X_MORPHO_GETFLOATVALUE(right));
        } else if (MORPHO_ISINTEGER(right)) {
            return X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) * X_MORPHO_GETINTEGERVALUE(right));
        }
    }

    // type error...
    return MORPHO_NIL;
}

dyn_var<value> op_mul(dyn_var<value> left, dyn_var<value> right, static_var<pe_t> ltype, static_var<pe_t> rtype) {
    if (ltype == PE_FLOAT_T) {
        if (rtype == PE_FLOAT_T)
            return X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) * X_MORPHO_GETFLOATVALUE(right));
        else if (rtype == PE_INT_T)
            return X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) * X_MORPHO_GETINTEGERVALUE(right));
        // else // (right_type is DYN) ...
    } else if (ltype == PE_INT_T) {
        if (rtype == PE_FLOAT_T)
            return X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) * X_MORPHO_GETFLOATVALUE(right));
        else if (rtype == PE_INT_T)
            return X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) * X_MORPHO_GETINTEGERVALUE(right));
        // else // right_type is DYN
    }

    return op_mul_dyn(left, right);
}

dyn_var<value> op_div_dyn(dyn_var<value> left, dyn_var<value> right) {
    if (MORPHO_ISFLOAT(left)) {
        if (MORPHO_ISFLOAT(right)) {
            return X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) / X_MORPHO_GETFLOATVALUE(right));
        } else if (MORPHO_ISINTEGER(right)) {
            return X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) / X_MORPHO_GETINTEGERVALUE(right));
        }
    } else if (MORPHO_ISINTEGER(left)) {
        if (MORPHO_ISFLOAT(right)) {
            return X_MORPHO_FLOAT(X_MORPHO_GETINTEGERVALUE(left) / X_MORPHO_GETFLOATVALUE(right));
        } else if (MORPHO_ISINTEGER(right)) {
            // TODO  I think this is actually not right
            return X_MORPHO_FLOAT(S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(left)) / X_MORPHO_GETINTEGERVALUE(right));
        }
    }

    // type error...
    return MORPHO_NIL;
}

dyn_var<value> op_div(dyn_var<value> left, dyn_var<value> right, static_var<pe_t> ltype, static_var<pe_t> rtype) {
    if (ltype == PE_FLOAT_T) {
        if (rtype == PE_FLOAT_T)
            return X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) / X_MORPHO_GETFLOATVALUE(right));
        else if (rtype == PE_INT_T)
            return X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) / X_MORPHO_GETINTEGERVALUE(right));
        // else // (right_type is DYN) ...
    } else if (ltype == PE_INT_T) {
        if (rtype == PE_FLOAT_T)
            return X_MORPHO_FLOAT(X_MORPHO_GETINTEGERVALUE(left) / X_MORPHO_GETFLOATVALUE(right));
        else if (rtype == PE_INT_T)
            return X_MORPHO_FLOAT(S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(left)) / X_MORPHO_GETINTEGERVALUE(right));
        // else // right_type is DYN
    }

    return op_div_dyn(left, right);
}

dyn_var<value> op_pow_dyn(dyn_var<value> left, dyn_var<value> right) {
    if (MORPHO_ISFLOAT(left)) {
        if (MORPHO_ISFLOAT(right))
            return X_MORPHO_FLOAT(runtime::pow(X_MORPHO_GETFLOATVALUE(left), 
                                               X_MORPHO_GETFLOATVALUE(right)));
        else if (MORPHO_ISINTEGER(right))
            return X_MORPHO_FLOAT(runtime::pow(X_MORPHO_GETFLOATVALUE(left), 
                                               S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(right))));
        // else // (right_type is DYN) ...
    } else if (MORPHO_ISINTEGER(left)) {
        if (MORPHO_ISFLOAT(right))
            return X_MORPHO_FLOAT(runtime::pow(S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(left)), 
                                               X_MORPHO_GETFLOATVALUE(right)));
        else if (MORPHO_ISINTEGER(right))
            return X_MORPHO_FLOAT(runtime::pow(S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(left)), 
                                               S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(right))));
        // else // right_type is DYN
    }

    // type error
    return MORPHO_NIL;
}

dyn_var<value> op_pow(dyn_var<value> left, dyn_var<value> right, static_var<pe_t> ltype, static_var<pe_t> rtype) {
    // if (ltype == PE_FLOAT_T) {
    //     if (rtype == PE_FLOAT_T)
    //         return X_MORPHO_FLOAT(runtime::pow(X_MORPHO_GETFLOATVALUE(left),
    //                                            X_MORPHO_GETFLOATVALUE(right) ));
    //     else if (rtype == PE_INT_T)
    //         return X_MORPHO_FLOAT(runtime::pow( X_MORPHO_GETFLOATVALUE(left),
    //                                             S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(right)) ));
    //     // else // (right_type is DYN) ...
    // } else if (ltype == PE_INT_T) {
    //     if (rtype == PE_FLOAT_T)
    //         return X_MORPHO_FLOAT(runtime::pow(S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(left)),
    //                                            X_MORPHO_GETFLOATVALUE(right) ));
    //     else if (rtype == PE_INT_T)
    //         return X_MORPHO_FLOAT(runtime::pow(S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(left)),
    //                                             S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(right)) ));
    //     // else // right_type is DYN
    // }

    return op_pow_dyn(left, right);
}



dyn_var<int> morpho_vm(const int n, const uint32_t instructions[], objectfunction *globalfn) {
    dyn_var<value[NUM_REGISTERS]> reg;
    dyn_var<value[NUM_GLOBALS]> globals;

    static_var<pe_t> reg_type[NUM_REGISTERS] = {PE_ERR_T};
    static_var<pe_t> globals_type[NUM_REGISTERS] = {PE_ERR_T};

    static_var<int32_t> a, b, c;
    static_var<int32_t> bc;
    static_var<int32_t> pc = 0;
    // static_var<int32_t> total_instructions = 10000;
/**/ 
    while (pc < n ) { // && total_instructions < MAX_INSTRUCTIONS) {
        bc = instructions[pc];
        switch (DECODE_OP(bc)) {

            case OP_NOP:
                break;

            case OP_MOV:
                a=DECODE_A(bc); b=DECODE_B(bc);
                reg[a] = reg[b];
                reg_type[a] = reg_type[b];
                break;

            case OP_LCT:
                a=DECODE_A(bc); b=DECODE_Bx(bc);
                reg[a] = globalfn->konst.data[b];
                reg_type[a] = value2petype(globalfn->konst.data[b]);
                break;

            case OP_ADD:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);

                reg_type[a] = integralBinOpTypeRule(reg_type[b], reg_type[c]);
                // pair of obj/dyns exception for string on string case
                if ((reg_type[b] & (PE_OBJ_T | PE_DYN_T)) || (reg_type[b] & (PE_OBJ_T | PE_DYN_T)) ) {
                    reg_type[a] = PE_DYN_T;
                }
                if (reg_type[a] == PE_ERR_T) {
                    runtime::printerr("Type error in OP_ADD");
                    return EXIT_FAILURE;
                }
 
                reg[a] = op_add(reg[b], reg[c], reg_type[b], reg_type[c]);
                break;

            case OP_SUB:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);

                reg_type[a] = integralBinOpTypeRule(reg_type[b], reg_type[c]);
                if (reg_type[a] == PE_ERR_T) {
                    runtime::printerr("Type error in OP_SUB");
                    return EXIT_FAILURE;
                }

                reg[a] = op_sub(reg[b], reg[c], reg_type[b], reg_type[c]);
                break;

            case OP_MUL:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);

                reg_type[a] = integralBinOpTypeRule(reg_type[b], reg_type[c]);
                if (reg_type[a] == PE_ERR_T) {
                    runtime::printerr("Type error in OP_MUL");
                    return EXIT_FAILURE;
                }

                reg[a] = op_mul(reg[b], reg[c], reg_type[b], reg_type[c]);
                break;

            case OP_DIV:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);

                reg_type[a] = integralBinOpTypeRule(reg_type[b], reg_type[c]);
                if (reg_type[a] == PE_ERR_T) {
                    runtime::printerr("Type error in OP_DIV");
                    return EXIT_FAILURE;
                }
                // div can actually only return float, but I wanna reuse the binop fn
                reg_type[a] = reg_type[a] == PE_DYN_T ? PE_DYN_T : PE_FLOAT_T;

                reg[a] = op_div(reg[b], reg[c], reg_type[b], reg_type[c]);
                break;

            case OP_POW: //POW
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);

                reg_type[a] = integralBinOpTypeRule(reg_type[b], reg_type[c]);
                if (reg_type[a] == PE_ERR_T) {
                    runtime::printerr("Type error in OP_DIV");
                    return EXIT_FAILURE;
                }
                reg_type[a] = reg_type[a] == PE_DYN_T ? PE_DYN_T : PE_FLOAT_T;

                // TODO!
                reg[a] = op_pow(reg[b], reg[c], reg_type[b], reg_type[c]);

                // OPREDIRECT(powselector, powrselector, a);
                // OPERROR("Exponentiate")
                break;
            case OP_EQ:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);

                // TODO replace with extendedcompareval
                reg[a] = X_MORPHO_BOOL(!x_morpho_extendedcomparevalue(reg[b], reg[c], reg_type[b], reg_type[c]));
                reg_type[a] = PE_BOOL_T;
                break;
            case OP_NEQ:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);

                reg[a] = X_MORPHO_BOOL(x_morpho_extendedcomparevalue(reg[b], reg[c], reg_type[b], reg_type[c]));
                reg_type[a] = PE_BOOL_T;
                break;
            case OP_NOT:
                a=DECODE_A(bc); b=DECODE_B(bc);
                reg_type[a] = PE_BOOL_T;

                if (reg_type[b] == PE_BOOL_T) {
                    reg[a] = X_MORPHO_BOOL(!X_MORPHO_GETBOOLVALUE(reg[b]));
                    break;
                } else if (reg_type[b] == PE_NIL_T) {
                    reg[a] = X_MORPHO_BOOL(true);
                    break;
                } else if (reg_type[b] != PE_DYN_T) {
                    reg[a] = X_MORPHO_BOOL(false);
                    break;
                }

                if (MORPHO_ISBOOL(reg[b])) {
                    reg[a] = X_MORPHO_BOOL(!X_MORPHO_GETBOOLVALUE(reg[b]));
                } else if (MORPHO_ISNIL(reg[b])) {
                    reg[a] = X_MORPHO_BOOL(MORPHO_ISNIL(reg[b]));
                }
                reg[a] = X_MORPHO_BOOL(false);

                break;
            case OP_LT: //LT
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);

                // TODO: Type Errors
                // if ( !( (MORPHO_ISFLOAT(left) || MORPHO_ISINTEGER(left)) &&
                //     (MORPHO_ISFLOAT(right) || MORPHO_ISINTEGER(right)) ) ) {
                //     OPERROR("Compare");
                // }

                reg[a] = X_MORPHO_BOOL(x_morpho_extendedcomparevalue(reg[b], reg[c], reg_type[b], reg_type[c]) > MORPHO_EQUAL);
                reg_type[a] = PE_BOOL_T;
                break;
            case OP_LE: //LT
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                // TODO: Type Errors
                // if ( !( (MORPHO_ISFLOAT(left) || MORPHO_ISINTEGER(left)) &&
                //        (MORPHO_ISFLOAT(right) || MORPHO_ISINTEGER(right)) ) ) {
                //     OPERROR("Compare");
                // }
                reg[a] = X_MORPHO_BOOL(x_morpho_extendedcomparevalue(reg[b], reg[c], reg_type[b], reg_type[c]) >= MORPHO_EQUAL);
                reg_type[a] = PE_BOOL_T;
                break;
            case OP_B: // B
                b=DECODE_sBx(bc);
                pc+=b;
                break;
            case OP_BIF: // BIF
                a=DECODE_A(bc);

                if (reg_type[a] == PE_BOOL_T) {
                    if (MORPHO_GETBOOLVALUE(reg[a])) pc+=DECODE_sBx(bc);
                    break;
                } else if (!(reg_type[a] & (PE_NIL_T | PE_DYN_T))) {
                    pc+=DECODE_sBx(bc);
                    break;
                }
                if (X_MORPHO_ISTRUE(reg[a])) pc+=DECODE_sBx(bc);
                break;
            case OP_BIFF: // BIFF
                a=DECODE_A(bc);

                if (reg_type[a] == PE_NIL_T) {
                    pc+=DECODE_sBx(bc);
                    break;
                } else if (reg_type[a] == PE_BOOL_T) {
                    if (!MORPHO_GETBOOLVALUE(reg[a])) pc+=DECODE_sBx(bc);
                    break;
                }
                if (X_MORPHO_ISFALSE(reg[a])) pc+=DECODE_sBx(bc);
                break;

            case OP_CALL: // CALL (no support for optional arguments yet)
                a=DECODE_A(bc); b=DECODE_B(bc);
                reg[a]=runtime::call(reg[a], b, reg + a);
                reg_type[a]= PE_DYN_T;
                break;
            case OP_LGL: // LGL
                a=DECODE_A(bc);
                b=DECODE_Bx(bc);
                reg[a]      = globals[b];
                reg_type[a] = globals_type[b];
                break;
            case OP_SGL: // SGL
                a=DECODE_A(bc);
                b=DECODE_Bx(bc);
                globals[b]      = reg[a];
                globals_type[b] = reg_type[a];
                break;
            case OP_PRINT: // PRINT
                a=DECODE_A(bc);
                runtime::print(reg[a]);
                break;
            case OP_END: // END
                return EXIT_SUCCESS;
            default:
                runtime::printerr("Encountered unimplemented instruction. Exiting.");
                return EXIT_FAILURE;
        }

        pc++;
        // total_instructions++;
    } //*/
    runtime::printerr("Program counter exceeded bytecode buffer. Exiting.");
    return EXIT_FAILURE;
}