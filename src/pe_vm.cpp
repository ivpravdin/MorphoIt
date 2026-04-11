#include "pe_vm.h"

#include "value.h"
#include "builtin.h"

#include "builder/dyn_var.h"
#include "builder/static_var.h"

#include "pe_vm_consts.h"

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;
using std::vector;

dyn_var<value> morpho_vm_rec(
    const int n,
    const instruction * const instructions,
    const objectfunction * const globalfn,
    const dyn_var<value> args[]
) {
    dyn_var<value[PE_NUM_REGS]> reg = builder::with_name("reg", true);
    const size_t total_args = globalfn->nargs + globalfn->nopt;

    // initialize regs with args
    reg[0] = MORPHO_OBJECT(globalfn);
    for (static_var<size_t> i = 0; i < total_args; i) {
        reg[i + 1] = args[i + 1];
    }

    dyn_var<value[PE_NUM_GLOBALS]> globals = builder::with_name(PE_GLOBALS);
    dyn_var<value> left = builder::with_name("left", true), right = builder::with_name("right", true);

    static_var<instruction> pc = globalfn->entry;

    while (pc < n) {
        const instruction bc = instructions[pc];
        const int32_t a = DECODE_A(bc), b = DECODE_B(bc), c = DECODE_C(bc), bx = DECODE_Bx(bc), sbx = DECODE_sBx(bc);
        switch (DECODE_OP(bc)) {
            case OP_NOP:
                break;

            case OP_MOV:
                reg[a] = reg[b];
                break;

            case OP_LCT:
                if (MORPHO_ISFUNCTION(globalfn->konst.data[bx])) {
                    const objectfunction *const fn = MORPHO_GETFUNCTION(globalfn->konst.data[bx]);
                    /* IF we don't have syntax tree of function f_xxxxxx:
                        generate said tree and generate C code definition of f_xxxxxx
                       ENDIF
        
                        Get it to generate C code according to "reg[b] = f_xxxxxx"
                        Do I care about static registers anymore? HELL NO!!!

                        SO i also need to generate a struct wrapper around the function so that
                        it is recognizable as a function?
                     */

                    // this is less solid than I thought: name can be the emptystring, so there could be naming conflicts here
                    dyn_var<uintptr_t> fn_ptr = builder::with_name( std::string("user_morpho_") + MORPHO_GETCSTRING(fn->name) );
                    reg[a] = X_MORPHO_OBJECT(fn_ptr);
                } else {
                    reg[a] = globalfn->konst.data[bx];
                }
                break;

            case OP_ADD:
                left = reg[b], right = reg[c];
                reg[a] = runtime::op_add(left, right);
                break;

            case OP_SUB:
                left = reg[b], right = reg[c];

                reg[a] = runtime::op_sub(left, right);
                break;

            case OP_MUL:
                left = reg[b], right = reg[c];
                reg[a] = runtime::op_mul(left, right);

                break;

            case OP_DIV:
                left = reg[b], right = reg[c];
                reg[a] = runtime::op_div(left, right);

                break;

            case OP_POW: //POW
                left = reg[b];
                right = reg[c];
                reg[a] = runtime::op_pow(left, right);

                break;

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

                break;
            case OP_LT: //LT
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(runtime::morpho_extendedcomparevalue(left, right) > MORPHO_EQUAL);
                break;
            case OP_LE: //LT
                left = reg[b];
                right = reg[c];

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
                // // runtime::print(reg[a]);          // print fn at runtime
                // // object_print(NULL, reg_stat[a]); // print fn at PE time
    
                // // in theory should also handle closures
                // if (MORPHO_ISFUNCTION(reg_stat[a])) {
                //     const objectfunction *func = MORPHO_GETFUNCTION(reg_stat[a]);
                //     // TODO: this does not handle functions as returned values
                //     dyn_var<value[PE_NUM_REGS]> regswithargs;
                //     static_var<value> regswithargs_stat[PE_NUM_REGS];
                //     // r0 = function object
                //     // I could pass it reg_stat but
                //     // that would be the literal pointer to the function
                //     regswithargs[0] = reg[a]; 
                //     regswithargs_stat[0] = reg_stat[a]; 
                //     // r1..rn = args 1..n
                //     const int32_t n_opt_args = b;
                //     const int32_t n_pos_args = c;
                //     for (static_var<size_t> i = 0; i < n_opt_args + n_pos_args; i++) {
                //         regswithargs[i + 1] = reg[a + 1 + i];
                //         regswithargs_stat[i + 1] = reg_stat[a + 1 + i];
                //     }
                //     // Zero out rest of static regs
                //     for (static_var<size_t> i = n_opt_args + n_pos_args + 1; i < PE_NUM_REGS; i++) {
                //         regswithargs_stat[i] = MORPHO_NIL;
                //     }

                //      /* * reg[b] = runtime::f
                //      * Is there ANY way to get static vars out of this bitch?
                //      * I don't think so? But we can make a hash table that converts function objects to 
                //      * actual functions. Is there any point then in inlining?
                //     */

                //     vm_return retv = morpho_vm_rec(
                //         n,
                //         instructions,
                //         func,
                //         globals,
                //         globals_stat,
                //         regswithargs,
                //         regswithargs_stat
                //     );
                //     // reg[a] = retv.ret_dyn;
                //     // reg_stat[a] = retv.ret_stat;
                // } else {
                    reg[a]=runtime::call(left, b, reg + a);
                // }
                break;
            case OP_RETURN:
                // ret_stat SHOULD BE A VALID FUNCTION OR NIL
                // ASSUMING reg_stat IS INITIALIZED TO BE ALL NIL
                // and we do want it to be well-initialized for static-tagging
                if (a>0)
                    return reg[b];
                else
                    return MORPHO_NIL;
                break;
            case OP_LGL: // LGL
                reg[a]=globals[bx];
                break;
            case OP_SGL: // SGL
                globals[bx]=reg[a];
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

dyn_var<value> morpho_vm(
    const int n,
    const instruction *const instructions,
    const objectfunction *const globalfn
) {
    // declaration of globals is handled in header.c, no other way to make it
    // actually global, apparently
    return morpho_vm_rec(
        n,
        instructions,
        globalfn,
        NULL // top level has no args?
    );
}