#include "pe_vm.h"

#include "value.h"
#include "builtin.h"

#include "builder/dyn_var.h"
#include "builder/static_var.h"

// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;
using std::vector;

typedef struct vm_return {
    dyn_var<value> ret_dyn;
    const value ret_stat;
} vm_return;

// I don't want this to be a method because that promotes the struct to a class
// and there is some difference between raw struct "aggregate types" and classes
// ...I think
inline vm_return vm_return_with_val(const value v) {
    return vm_return { .ret_dyn = (v) , .ret_stat = (v) };
}

constexpr size_t NUM_REGS = 255;
constexpr size_t NUM_GLOBALS = 100;


vm_return morpho_vm_rec(
    const int n,
    const instruction * const instructions,
    const objectfunction * const globalfn,
    dyn_var<value[NUM_GLOBALS]> &globals,
    static_var<value> globals_stat[],
    dyn_var<value[NUM_REGS]> &reg,
    static_var<value> reg_stat[]
) {
    // dyn_var<value> left = builder::with_name("left", true), right = builder::with_name("right", true);
    dyn_var<value> left, right;

    // const int nregs = globalfn->nregs;

    // This doesn't seem to cause infinite loops
    // static_var<value> globals_stat[NUM_GLOBALS];
    // for (static_var<size_t> i = 0; i < NUM_GLOBALS; i++) {
    //     globals_stat[i] = MORPHO_NIL;
    // }

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
                // runtime::print(reg[a]);          // print fn at runtime
                // object_print(NULL, reg_stat[a]); // print fn at PE time
    
                // in theory should also handle closures
                if (MORPHO_ISFUNCTION(reg_stat[a])) {
                    const objectfunction *func = MORPHO_GETFUNCTION(reg_stat[a]);
                    // TODO: this does not handle functions as returned values
                    dyn_var<value[NUM_REGS]> regswithargs;
                    static_var<value> regswithargs_stat[NUM_REGS];
                    // r0 = function object
                    // I could pass it reg_stat but
                    // that would be the literal pointer to the function
                    regswithargs[0] = reg[a]; 
                    regswithargs_stat[0] = reg_stat[a]; 
                    // r1..rn = args 1..n
                    const int32_t n_opt_args = b;
                    const int32_t n_pos_args = c;
                    for (static_var<size_t> i = 0; i < n_opt_args + n_pos_args; i++) {
                        regswithargs[i + 1] = reg[a + 1 + i];
                        regswithargs_stat[i + 1] = reg_stat[a + 1 + i];
                    }
                    // Zero out rest of static regs
                    for (static_var<size_t> i = n_opt_args + n_pos_args + 1; i < NUM_REGS; i++) {
                        regswithargs_stat[i] = MORPHO_NIL;
                    }

                    vm_return retv = morpho_vm_rec(
                        n,
                        instructions,
                        func,
                        globals,
                        globals_stat,
                        regswithargs,
                        regswithargs_stat
                    );
                    reg[a] = retv.ret_dyn;
                    reg_stat[a] = retv.ret_stat;
                } else {
                    reg[a]=runtime::call(left, b, reg + a);
                }
                break;
            case OP_RETURN:
                // ret_stat SHOULD BE A VALID FUNCTION OR NIL
                // ASSUMING reg_stat IS INITIALIZED TO BE ALL NIL
                // and we do want it to be well-initialized for static-tagging
                if (a>0)
                    return vm_return { .ret_dyn = reg[b] , .ret_stat = reg_stat[b] };
                else
                    return vm_return_with_val(MORPHO_NIL);
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
                return vm_return_with_val(EXIT_SUCCESS);
            default:
                runtime::printerr("Encountered unimplemented instruction. Exiting.");
                return vm_return_with_val(EXIT_FAILURE);
        }

        pc++;
    }
    runtime::printerr("Program counter exceeded bytecode buffer. Exiting.");
    return vm_return_with_val(EXIT_FAILURE);
}

dyn_var<value> morpho_vm(
    const int n,
    const instruction *const instructions,
    const objectfunction *const globalfn
) {
    dyn_var<value[NUM_GLOBALS]> globals = builder::with_name("globals", true);
    dyn_var<value[NUM_REGS]> reg;

    // const int nregs = globalfn->nregs;
    // vector<static_var<value>> reg_stat(nregs);
    // for (static_var<size_t> i = 0; i < nregs; i++) {
    //     reg_stat[i] = MORPHO_NIL;
    // }
    static_var<value> globals_stat[NUM_GLOBALS];
    for (static_var<size_t> i = 0; i < NUM_GLOBALS; i++) {
        globals_stat[i] = MORPHO_NIL;
    }

    // this could be a vector of just the required number of regs?
    // Idk how variadic arguments play with this. But certainly there is an nregs
    // field in the function object
    // I think that's street legal
    static_var<value> reg_stat[NUM_REGS];
    for (static_var<size_t> i = 0; i < NUM_REGS; i++) {
        reg_stat[i] = MORPHO_NIL;
    }

    return morpho_vm_rec(
        n,
        instructions,
        globalfn,
        globals,
        globals_stat,
        reg,
        reg_stat
    ).ret_dyn;
}