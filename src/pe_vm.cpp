#include "pe_vm.h"

#include "value.h"
#include "builtin.h"

#include "builder/dyn_var.h"
#include "builder/static_var.h"

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

dyn_var<int> morpho_vm(const int n, const uint32_t instructions[], objectfunction *globalfn) {
    dyn_var<value[255]> reg;
    dyn_var<value> left, right;
    
    dyn_var<value[100]> globals;
    
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
                break;

            case OP_LCT:
                a=DECODE_A(bc); b=DECODE_Bx(bc);
                reg[a] = globalfn->konst.data[b];
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
                if (MORPHO_ISFLOAT(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) + X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) + X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                } else if (MORPHO_ISINTEGER(left)) {
                    if (MORPHO_ISFLOAT(right)) {
                        reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) + X_MORPHO_GETFLOATVALUE(right));
                        break;
                    } else if (MORPHO_ISINTEGER(right)) {
                        reg[a] = X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) + X_MORPHO_GETINTEGERVALUE(right));
                        break;
                    }
                } else if (MORPHO_ISSTRING(left) && MORPHO_ISSTRING(right)) {
                    reg[a] = object_concatenatestring(left, right);
                }
                break;
            /* OPCODE: SUBTRACT
             * 
             * To-Do:
             *      - Object operator redirection
             */
            case OP_SUB:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b], right = reg[c];

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

                // OPREDIRECT(powselector, powrselector, a);
                // OPERROR("Exponentiate")
                break;
            case OP_EQ:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(!x_morpho_extendedcomparevalue(left, right));
                break;
            case OP_NEQ:
                a=DECODE_A(bc); b=DECODE_B(bc); c=DECODE_C(bc);
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(x_morpho_extendedcomparevalue(left, right));
                break;
            case OP_NOT:
                a=DECODE_A(bc); b=DECODE_B(bc);
                left = reg[b];

                if (MORPHO_ISBOOL(left)) {
                    reg[a] = X_MORPHO_BOOL(!X_MORPHO_GETBOOLVALUE(left));
                } else {
                    reg[a] = X_MORPHO_BOOL(MORPHO_ISNIL(left));
                }
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
            case OP_LGL: // LGL
                a=DECODE_A(bc);
                b=DECODE_Bx(bc);
                reg[a]=globals[b];
                break;
            case OP_SGL: // SGL
                a=DECODE_A(bc);
                b=DECODE_Bx(bc);
                globals[b]=reg[a];
                break;
            case OP_PRINT: // PRINT
                a=DECODE_A(bc);
                left=reg[a];
                if (MORPHO_ISINTEGER(left)) {
                    runtime::printint( X_MORPHO_GETINTEGERVALUE(left) );
                } else if (MORPHO_ISFLOAT(left)) {
                    runtime::printfloat( X_MORPHO_GETFLOATVALUE(left) );
                } else if (MORPHO_ISBOOL(left)) {
                    runtime::printbool(X_MORPHO_GETBOOLVALUE(left));
                } else if (MORPHO_ISNIL(left)) {
                    runtime::printnil();
                } else if (MORPHO_ISOBJECT(left)) {
                    dyn_var<dyn_object *> objptr = X_MORPHO_GETOBJECT(left);
                    runtime::object_print(0, objptr);
                } else {
                    runtime::printerr("Unknown type to print.");
                }
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