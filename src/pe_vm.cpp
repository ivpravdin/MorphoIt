#include "pe_vm.h"

#include "value.h"
#include "builtin.h"

#include "builder/dyn_var.h"
#include "builder/static_var.h"

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

#define NUM_REGISTERS 255
#define NUM_GLOBALS 100
typedef enum {
    ERR = 0,
    NIL,
    BOOL,
    INT,
    FLOAT,
    CMPLX,
    OBJECT,
    DYN
} PE_Type;

PE_Type value2petype(value v) {
    switch MORPHO_GETTYPE(v) {
        case TAG_NIL: return PE_Type::NIL;
        case TAG_BOOL: return PE_Type::BOOL;
        case TAG_INT: return PE_Type::INT;
        case TAG_OBJ: return PE_Type::OBJECT;
        default: return PE_Type::FLOAT;
    };
}

PE_Type integralBinOpTypeRule(PE_Type lhs, PE_Type rhs) {
    if (lhs == PE_Type::DYN || rhs == PE_Type::DYN) {
        return PE_Type::DYN;
    }

    if (lhs == PE_Type::FLOAT && (rhs == PE_Type::FLOAT || rhs == PE_Type::INT)) {
            return PE_Type::FLOAT;
    } else if (lhs == PE_Type::INT) {
        if (rhs == PE_Type::FLOAT) {
            return PE_Type::FLOAT;
        } else if (rhs == PE_Type::INT) {
            return PE_Type::INT;
        }
    }

    return PE_Type::ERR;
}


dyn_var<int> morpho_vm(const int n, const uint32_t instructions[], objectfunction *globalfn) {
    dyn_var<value[NUM_REGISTERS]> reg;
    dyn_var<value[NUM_GLOBALS]> globals;

    static_var<PE_Type> reg_type[NUM_REGISTERS]     = { PE_Type::ERR };
    static_var<PE_Type> globals_type[NUM_REGISTERS] = { PE_Type::ERR };

    dyn_var<value> left, right;
    static_var<PE_Type> left_type, right_type;
    static_var<int32_t> a, b, c;
    static_var<int32_t> bc;
    static_var<int32_t> pc = 0;

    while (pc < n) {
        bc = instructions[pc];
        switch (DECODE_OP(bc)) {
            /* OPCODE: NO-OP
             * [COMPLETE]
             */
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
            /* OPCODE: ADD
             * 
             * To-Do:
             *      - Finish string concatenation?
             *      - Object operator redirection
             */
            case OP_ADD:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b], right = reg[c];
                left_type = reg_type[b], right_type = reg_type[c];
                reg[a] = runtime::add(left, right);
                reg_type[a] = integralBinOpTypeRule(left_type, right_type);
                break;
            /* OPCODE: SUBTRACT
             * 
             * To-Do:
             *      - Object operator redirection
             */
            case OP_SUB:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b], right = reg[c];
                // left_type = reg_type[b], right_type = reg_type[c];
                // reg_type[a] = integralBinOpTypeRule(left_type, right_type);

                if (MORPHO_ISFLOAT(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) - X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) - X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                } else if (MORPHO_ISINTEGER(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) - X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) - X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                }
                break;
            /* OPCODE: MULTIPLY
             * 
             * To-Do:
             *      - Object operator redirection
             */
            case OP_MUL:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b], right = reg[c];
                left_type = reg_type[b], right_type = reg_type[c];
                reg_type[a] = integralBinOpTypeRule(left_type, right_type);

                if (MORPHO_ISFLOAT(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) * X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) * X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                } else if (MORPHO_ISINTEGER(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) * X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) * X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                }
                break;
            /* OPCODE: DIVIDE
             * 
             * To-Do:
             *      - Object operator redirection
             */
            case OP_DIV:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b], right = reg[c];
                left_type = reg_type[b], right_type = reg_type[c];

                if (MORPHO_ISFLOAT(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) / X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) / X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                } else if (MORPHO_ISINTEGER(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) / X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) / X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                }
                break;
            /* OPCODE: POW
             * 
             * To-Do:
             *      - Verify implementation?
             *      - Object operator redirection
             */
            case OP_POW: //POW
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b];
                right = reg[c];

                if (MORPHO_ISFLOAT(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( runtime::pow(X_MORPHO_GETFLOATVALUE(left), X_MORPHO_GETFLOATVALUE(right)) );
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_FLOAT( runtime::pow(X_MORPHO_GETFLOATVALUE(left), S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(right)) ));
                        break;
                    }
                } else if (MORPHO_ISINTEGER(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( runtime::pow(S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(left)),
                                                            X_MORPHO_GETFLOATVALUE(right)) );
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = 
                            X_MORPHO_FLOAT( runtime::pow(S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(left)),
                                                        S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(right))));
                        break;
                    }
                }
                reg_type[a] = PE_Type::FLOAT;

                // OPREDIRECT(powselector, powrselector, a);
                // OPERROR("Exponentiate")
                break;
            case OP_EQ:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(!x_morpho_extendedcomparevalue(left, right));
                reg_type[a] = PE_Type::BOOL;
                break;
            case OP_NEQ:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(x_morpho_extendedcomparevalue(left, right));
                reg_type[a] = PE_Type::BOOL;
                break;
            case OP_NOT:
                a=DECODE_A(bc); b=DECODE_B(bc);
                left = reg[b];

                if (MORPHO_ISBOOL(left)) {
                    reg[a] = X_MORPHO_BOOL(!X_MORPHO_GETBOOLVALUE(left));
                } else {
                    reg[a] = X_MORPHO_BOOL(MORPHO_ISNIL(left));
                }
                reg_type[a] = PE_Type::BOOL;
                break;
            case OP_LT: //LT
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b];
                right = reg[c];

                // TODO: Type Errors
                // if ( !( (MORPHO_ISFLOAT(left) || MORPHO_ISINTEGER(left)) &&
                //     (MORPHO_ISFLOAT(right) || MORPHO_ISINTEGER(right)) ) ) {
                //     OPERROR("Compare");
                // }

                reg[a] = X_MORPHO_BOOL(x_morpho_extendedcomparevalue(left, right) > MORPHO_EQUAL);
                reg_type[a] = PE_Type::BOOL;
                break;
            case OP_LE: //LT
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b];
                right = reg[c];
                // TODO: Type Errors
                // if ( !( (MORPHO_ISFLOAT(left) || MORPHO_ISINTEGER(left)) &&
                //        (MORPHO_ISFLOAT(right) || MORPHO_ISINTEGER(right)) ) ) {
                //     OPERROR("Compare");
                // }
                reg[a] = X_MORPHO_BOOL(x_morpho_extendedcomparevalue(left, right) >= MORPHO_EQUAL);
                reg_type[a] = PE_Type::BOOL;
                break;
            case OP_B: // B
                b=DECODE_sBx(bc);
                pc+=b;
                break;
            case OP_BIF: // BIF
                a=DECODE_A(bc);
                left=reg[a];

                if (X_MORPHO_ISTRUE(left)) pc+=DECODE_sBx(bc);
                break;
            case OP_BIFF: // BIFF
                a=DECODE_A(bc);
                left=reg[a];

                if (X_MORPHO_ISFALSE(left)) pc+=DECODE_sBx(bc);
                break;

            case OP_CALL: // CALL (no support for optional arguments yet)
                a=DECODE_A(bc); b=DECODE_B(bc);
                left = reg[a];
                reg[a]=runtime::call(left, b, reg + a);
                reg_type[a]= PE_Type::DYN;
                break;
            case OP_LGL: // LGL
                a=DECODE_A(bc);
                b=DECODE_Bx(bc);
                reg[a]=globals[b];
                reg_type[a] = globals_type[b];
                break;
            case OP_SGL: // SGL
                a=DECODE_A(bc);
                b=DECODE_Bx(bc);
                globals[b]=reg[a];
                globals_type[b] = reg_type[a];
                break;
            case OP_PRINT: // PRINT
                a=DECODE_A(bc);
                left=reg[a];
                runtime::print(left);
                break;
            case OP_END: // END
                return EXIT_SUCCESS;    
            default:
                runtime::printerr("Encountered unimplemented instruction. Exiting.");
                return EXIT_FAILURE;
        }

        pc++;
    }
    runtime::printerr("Program counter exceeded bytecode buffer. Exiting.");
    return EXIT_FAILURE;
}