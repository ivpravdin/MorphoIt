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


dyn_var<value> morpho_vm_rec(
    const int n,
    const instruction * const instructions,
    const objectfunction * const globalfn,
    dyn_var<value[NUM_GLOBALS]> &globals,
    dyn_var<value[NUM_REGS]> &reg
) {
    // dyn_var<value> left = builder::with_name("left", true), right = builder::with_name("right", true);
    dyn_var<value> left, right;


    // This doesn't seem to cause infinite loops
    static_var<value> reg_stat[NUM_REGS];
    for (static_var<size_t> i = 0; i < NUM_REGS; i++) {
        reg_stat[i] = MORPHO_NIL;
    }
    static_var<value> globals_stat[NUM_GLOBALS];
    for (static_var<size_t> i = 0; i < NUM_GLOBALS; i++) {
        globals_stat[i] = MORPHO_NIL;
    }

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
                break;
            /* OPCODE: MULTIPLY
             * 
             * To-Do:
             *      - Object operator redirection
             */
            case OP_MUL:
                left = reg[b], right = reg[c];
                reg[a] = runtime::op_mul(left, right);

                break;
            /* OPCODE: DIVIDE
             * 
             * To-Do:
             *      - Object operator redirection
             */
            case OP_DIV:
                left = reg[b], right = reg[c];
                reg[a] = runtime::op_div(left, right);

                break;
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
                runtime::print(reg[a]);
                // in theory should also handle closures
                if (MORPHO_ISFUNCTION(reg_stat[a])) {
                    const objectfunction *func = MORPHO_GETFUNCTION(reg_stat[a]);
                    // TODO: this does not handle functions as returned values
                    dyn_var<value[NUM_REGS]> regswithargs;
                    // r0 = function object
                    // I could pass it reg_stat but
                    // that would be the literal pointer to the function
                    regswithargs[0] = reg[a]; 
                    // r1..rn = args 1..n
                    const int nargs = func->nargs;
                    for (static_var<int> i = 0; i < nargs; i++) {
                        regswithargs[i + 1] = reg[a + 1 + i];
                    }

                    reg[a] = morpho_vm_rec(n, instructions, func, globals, regswithargs);
                } else {
                    reg[a]=runtime::call(left, b, reg + a);
                }
                break;
            case OP_RETURN:
                if (a>0) {
                    // so for proper fn behavior this should definitely return
                    // a static value as well with a function, if there is one
                    // but to implement this is highly highly annoying
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

dyn_var<value> morpho_vm(const int n, const instruction *const instructions, const objectfunction *const globalfn) {
    dyn_var<value[NUM_GLOBALS]> globals; // = builder::with_name("globals", true);
    dyn_var<value[NUM_REGS]> reg;
    return morpho_vm_rec(n, instructions, globalfn, globals, reg);
}