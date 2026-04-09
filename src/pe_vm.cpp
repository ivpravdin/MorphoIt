#include "pe_vm.h"

#include "value.h"
#include "builtin.h"

#include "builder/dyn_var.h"
#include "builder/static_var.h"

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;

constexpr size_t NUM_REGS = 255;
constexpr size_t NUM_GLOBALS = 100;

dyn_var<value> morpho_vm(const int n, const instruction * const instructions, const objectfunction * const globalfn) {
    dyn_var<value[NUM_REGS]> reg;
    // dyn_var<value> left = builder::with_name("left", true), right = builder::with_name("right", true);
    dyn_var<value> left, right;

    dyn_var<value[NUM_GLOBALS]> globals; // = builder::with_name("globals", true);

    // WARNING: BUILDIT SAYS THESE SHOULD BE STATIC SINCE THEY MUTATE BUT THAT CAUSES
    // INFINITE LOOPING ISSUES
    value reg_stat[NUM_REGS];
    value globals_stat[NUM_GLOBALS];

    static_var<instruction> pc = globalfn->entry;

    while (pc < n) {
        const instruction bc = instructions[pc];
        const int32_t a = DECODE_A(bc), b = DECODE_B(bc), c = DECODE_C(bc), bx = DECODE_Bx(bc), sbx = DECODE_sBx(bc);
        switch (DECODE_OP(bc)) {
            case OP_NOP:
                break;

            case OP_MOV:
                reg[a] = reg[b];
                if (MORPHO_ISFUNCTION(reg_stat[b]))
                    reg_stat[a] = reg_stat[b];
                break;

            case OP_LCT:
                reg[a] = globalfn->konst.data[bx];
                if (MORPHO_ISFUNCTION(globalfn->konst.data[bx])) {
                    reg_stat[a] = globalfn->konst.data[bx];
                }
                break;
            /* OPCODE: ADD
             * 
             * To-Do:
             *      - Finish string concatenation?
             *      - Object operator redirection
             */
            case OP_ADD:
                left = reg[b], right = reg[c];
                reg[a] = runtime::op_add(left, right);
                break;
            /* OPCODE: SUBTRACT
             * 
             * To-Do:
             *      - Object operator redirection
             */
            case OP_SUB:
                left = reg[b], right = reg[c];

                reg[a] = runtime::op_sub(left, right);
                // if (MORPHO_ISFLOAT(left)) {
                //     if (MORPHO_ISFLOAT(right)) {
                //         reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) - X_MORPHO_GETFLOATVALUE(right));
                //         break;
                //     } else if (MORPHO_ISINTEGER(right)) {
                //         reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) - X_MORPHO_GETINTEGERVALUE(right));
                //         break;
                //     }
                // } else if (MORPHO_ISINTEGER(left)) {
                //     if (MORPHO_ISFLOAT(right)) {
                //         reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) - X_MORPHO_GETFLOATVALUE(right));
                //         break;
                //     } else if (MORPHO_ISINTEGER(right)) {
                //         reg[a] = X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) - X_MORPHO_GETINTEGERVALUE(right));
                //         break;
                //     }
                // }
                break;
            /* OPCODE: MULTIPLY
             * 
             * To-Do:
             *      - Object operator redirection
             */
            case OP_MUL:
                left = reg[b], right = reg[c];
                reg[a] = runtime::op_mul(left, right);

                // if (MORPHO_ISFLOAT(left)) {
                //     if (MORPHO_ISFLOAT(right)) {
                //         reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) * X_MORPHO_GETFLOATVALUE(right));
                //         break;
                //     } else if (MORPHO_ISINTEGER(right)) {
                //         reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) * X_MORPHO_GETINTEGERVALUE(right));
                //         break;
                //     }
                // } else if (MORPHO_ISINTEGER(left)) {
                //     if (MORPHO_ISFLOAT(right)) {
                //         reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) * X_MORPHO_GETFLOATVALUE(right));
                //         break;
                //     } else if (MORPHO_ISINTEGER(right)) {
                //         reg[a] = X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) * X_MORPHO_GETINTEGERVALUE(right));
                //         break;
                //     }
                // }
                break;
            /* OPCODE: DIVIDE
             * 
             * To-Do:
             *      - Object operator redirection
             */
            case OP_DIV:
                left = reg[b], right = reg[c];
                reg[a] = runtime::op_div(left, right);

            //     if (MORPHO_ISFLOAT(left)) {
            //         if (MORPHO_ISFLOAT(right)) {
            //             reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) / X_MORPHO_GETFLOATVALUE(right));
            //             break;
            //         } else if (MORPHO_ISINTEGER(right)) {
            //             reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETFLOATVALUE(left) / X_MORPHO_GETINTEGERVALUE(right));
            //             break;
            //         }
            //     } else if (MORPHO_ISINTEGER(left)) {
            //         if (MORPHO_ISFLOAT(right)) {
            //             reg[a] = X_MORPHO_FLOAT( X_MORPHO_GETINTEGERVALUE(left) / X_MORPHO_GETFLOATVALUE(right));
            //             break;
            //         } else if (MORPHO_ISINTEGER(right)) {
            //             reg[a] = X_MORPHO_INTEGER( X_MORPHO_GETINTEGERVALUE(left) / X_MORPHO_GETINTEGERVALUE(right));
            //             break;
            //         }
            //     }
            //     break;
            /* OPCODE: POW
             * 
             * To-Do:
             *      - Verify implementation?
             *      - Object operator redirection
             */
            case OP_POW: //POW
                left = reg[b];
                right = reg[c];
                reg[a] = runtime::op_pow(left, right);

            //     if (MORPHO_ISFLOAT(left)) {
            //         if (MORPHO_ISFLOAT(right)) {
            //             reg[a] = X_MORPHO_FLOAT( runtime::pow(X_MORPHO_GETFLOATVALUE(left), X_MORPHO_GETFLOATVALUE(right)) );
            //             break;
            //         } else if (MORPHO_ISINTEGER(right)) {
            //             reg[a] = X_MORPHO_FLOAT( runtime::pow(X_MORPHO_GETFLOATVALUE(left), S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(right)) ));
            //             break;
            //         }
            //     } else if (MORPHO_ISINTEGER(left)) {
            //         if (MORPHO_ISFLOAT(right)) {
            //             reg[a] = X_MORPHO_FLOAT( runtime::pow(S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(left)),
            //                                                 X_MORPHO_GETFLOATVALUE(right)) );
            //             break;
            //         } else if (MORPHO_ISINTEGER(right)) {
            //             reg[a] = 
            //                 X_MORPHO_FLOAT( runtime::pow(S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(left)),
            //                                             S_CAST_DYN_VAR(double, X_MORPHO_GETINTEGERVALUE(right))));
            //             break;
            //         }
            //     }

            //     // OPREDIRECT(powselector, powrselector, a);
            //     // OPERROR("Exponentiate")
            //     break;
            case OP_EQ:
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(!runtime::morpho_extendedcomparevalue(left, right));
                break;
            case OP_NEQ:
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(runtime::morpho_extendedcomparevalue(left, right));
                break;
            case OP_NOT:
                left = reg[b];

                reg[a] = runtime::op_not(left);

                // if (MORPHO_ISBOOL(left)) {
                //     reg[a] = X_MORPHO_BOOL(!MORPHO_GETBOOLVALUE(left));
                // } else {
                //     reg[a] = X_MORPHO_BOOL(MORPHO_ISNIL(left));
                // }
                break;
            case OP_LT: //LT
                left = reg[b];
                right = reg[c];

                // TODO: Type Errors
                // if ( !( (MORPHO_ISFLOAT(left) || MORPHO_ISINTEGER(left)) &&
                //     (MORPHO_ISFLOAT(right) || MORPHO_ISINTEGER(right)) ) ) {
                //     OPERROR("Compare");
                // }

                reg[a] = X_MORPHO_BOOL(runtime::morpho_extendedcomparevalue(left, right) > MORPHO_EQUAL);
                break;
            case OP_LE: //LT
                left = reg[b];
                right = reg[c];
                // TODO: Type Errors
                // if ( !( (MORPHO_ISFLOAT(left) || MORPHO_ISINTEGER(left)) &&
                //        (MORPHO_ISFLOAT(right) || MORPHO_ISINTEGER(right)) ) ) {
                //     OPERROR("Compare");
                // }
                reg[a] = X_MORPHO_BOOL(runtime::morpho_extendedcomparevalue(left, right) >= MORPHO_EQUAL);
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
                if (!MORPHO_ISBUILTINFUNCTION(reg_stat[a]) && !MORPHO_ISMETAFUNCTION(reg_stat[a])) {
                    // TODO: this does not handle functions as returned values
                    reg[a] = morpho_vm(n, instructions, MORPHO_GETFUNCTION(reg_stat[a]));
                } else {
                    reg[a]=runtime::call(left, b, reg + a);
                }
                break;
            case OP_RETURN:
                if (a>0) {
                    return reg[b];
                } else {
                    return MORPHO_NIL; /* No return value; returns nil */
                }
                break;
            case OP_LGL: // LGL
                reg[a]=globals[bx];
                if (MORPHO_ISFUNCTION(globals_stat[bx])) {
                    reg_stat[a] = globals_stat[bx];
                }
                break;
            case OP_SGL: // SGL
                globals[bx]=reg[a];
                if (MORPHO_ISFUNCTION(reg_stat[a])) {
                    globals_stat[bx] = reg_stat[a];
                }
                break;
            case OP_PRINT: // PRINT
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

/** @brief Performs a function call
 *  @details A function call involves:
 *           1. Saving the program counter, register index and stacksize to the callframe stack;
 *           2. Advancing the frame pointer;
 *           3. Extracting the function from a closure if necessary;
 *           4. Expanding the stack if necessary
 *             Copy arguments to appropriate registers
 *           5. Loading the constant table from the function definition
 *           6. Shifting the register base
 *           7. Moving the program counter to the function
 * @param[in]  v                         The virtual machine
 * @param[in]  fn                       Function to call
 * @param[in]  regcall            rshift becomes r0 in the new call frame
 * @param[in]  nargs                number of positional arguments
 * @param[in]  nopt                  number of optional arguments
 * @param[in]  args                  pointer to the list of args
 * @param[out] pc                       program counter, updated
 * @param[out] reg                     register/stack pointer, updated */
// static inline bool vm_call(
//     vm *v,
//     const value fn,
//     const unsigned int regcall,
//     const unsigned int nargs,
//     const unsigned int nopt,
//     const value *args,
//     static_var<instruction> *pc,
//     dyn_var<value **>reg
// ) {
//     const objectfunction *func = MORPHO_GETFUNCTION(fn);
//     static_var<bool> argsonstack=true;
//     static_var<ptrdiff_t> aoffset=0;

//     const value *arglist=args;
//     if (arglist) {
//         /** Determine whether the arguments provided are on the stack or not */
//         argsonstack=(v->stack.data && arglist>v->stack.data && arglist<v->stack.data+v->stack.capacity);
//     } else { /** Otherwise use the registers after regcall */
//         arglist=(*reg)+regcall+1;
//     }
    
//     if (argsonstack) aoffset=arglist-v->stack.data; /** If args on stack retain where they are */
    
//     /* In the old frame... */
//     v->fp->pc=*pc; /* Save the program counter */
//     v->fp->stackcount=v->fp->function->nregs+(unsigned int) v->fp->roffset; /* Store the stacksize */
//     v->fp->returnreg=regcall; /* Store the return register */
//     unsigned int oldnregs = v->fp->function->nregs; /* Get the old number of registers */

//     if (v->fp==v->fpmax) { // Detect stack overflow
//         vm_runtimeerror(v, (*pc) - v->instructions, VM_STCKOVFLW);
//         return false;
//     }
//     v->fp++; /* Advance frame pointer */
//     v->fp->pc=*pc; /* We will also store the program counter in the new frame;
//                       this will be used to detect whether the VM should return on OP_RETURN */
// #ifdef MORPHO_PROFILER
//     v->fp->inbuiltinfunction=NULL;
// #endif

//     if (MORPHO_ISCLOSURE(fn)) {
//         objectclosure *closure=MORPHO_GETCLOSURE(fn); /* Closure object in use */
//         func=closure->func;
//         v->fp->closure=closure;
//     } else {
//         v->fp->closure=NULL;
//     }

//     v->fp->ret=false; /* Interpreter should not return from this frame */
//     v->fp->function=func; /* Store the function */

//     /* Do we need to expand the stack? */
//     if (v->stack.count+func->nregs>v->stack.capacity) {
//         vm_expandstack(v, reg, func->nregs); /* Expand the stack */
//     } else {
//         v->stack.count+=func->nregs;
//     }

//     v->konst = func->konst.data; /* Load the constant table */
//     *reg += oldnregs; /* Shift the register frame */
//     v->fp->roffset=*reg-v->stack.data; /* Store the register index */
    
//     /* Copy arguments into new register window */
//     if (argsonstack) {
//         arglist = v->stack.data+aoffset; // If they were on the stack, the pointer may be invalid so update it.
//         (*reg)[0] = arglist[-1]; // Copy the caller into r0
//     } else {
//         (*reg)[0] = fn;
//     }
    
//     for (unsigned int i=0; i<nargs; i++) (*reg)[i+1] = arglist[i];

//     int nvarg=0;
//     if (func->varg>=0) {
//         if (!vm_vargs(v, (*pc) - v->instructions, func, nargs, arglist, *reg)) return false;
//         nvarg=1;
//     } else if (func->nargs!=nargs) {
//         vm_runtimeerror(v, (*pc) - v->instructions, VM_INVALIDARGS, func->nargs, nargs);
//         return false;
//     }
    
//     /* Handle optional args */
//     if (func->opt.count>0) {
//         if (!vm_optargs(v, (*pc) - v->instructions, func, nopt, arglist+nargs, (*reg)+func->nargs+nvarg+1)) return false;
//     } else if (nopt>0) {
//         vm_runtimeerror(v, (*pc) - v->instructions, VM_NOOPTARG);
//         return false;
//     }

//     /* Zero out registers beyond args up to the top of the stack
//        This has to be fast: memset was too slow. Zero seems to be faster than MORPHO_NIL */
//     for (value *r = *reg + func->nregs-1; r > *reg + func->nargs + func->nopt + nvarg; r--) *r = MORPHO_INTEGER(0);

//     *pc=v->instructions+func->entry; /* Jump to the function */
//     return true;
// }