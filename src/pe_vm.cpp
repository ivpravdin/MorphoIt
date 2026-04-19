#include "pe_vm.h"

#include <exception>
#include "builder/dyn_var.h"
#include "builder/static_var.h"

#include "value.h"
#include "runtime.h"
#include "pe_vm_consts.h"


// Include the BuildIt types
using builder::dyn_var;
using builder::static_var;
using std::vector;
class UnimplementedInstructionException: public std::exception
{
  virtual const char* what() const throw()
  {
    return "Unimplemented instruction encountered.";
  }
};

class OutOfBoundsPCException: public std::exception
{
  virtual const char* what() const throw()
  {
    return "Program counter exceeded bytecode buffer.";
  }
};

dyn_var<value> morpho_vm_rec(
    dyn_var<value *> args, // all buildit examples have dyns first, idk why
    const int n,
    const instruction * const instructions,
    const objectfunction * const globalfn
) {
    // declare arr "reg" with PE_NUM_REGS elements
    dyn_var<value[PE_NUM_REGS]> reg = builder::with_name("reg", true);

    dyn_var<value[PE_NUM_GLOBALS]> globals = builder::with_name(PE_GLOBALS);
    dyn_var<value> left = builder::with_name("left", true), right = builder::with_name("right", true);

    static_var<value> reg_stat[PE_NUM_REGS];
    for (static_var<size_t> i = 0; i < PE_NUM_REGS; i++) {
        reg_stat[i] = MORPHO_NIL;
    }

    static_var<instruction> pc = globalfn->entry;

    // init'ing reg[0]
    // for now we'll just do this, since, e.g. the globalfn has a NULL name
    if (MORPHO_ISOBJECT(globalfn->name)) {
        dyn_var<userfn *> fn_ptr = builder::with_name(get_mangled_fn_name(globalfn));
        dyn_var<struct userfn_object> runtime_fn_obj;
        runtime_fn_obj.type = objectfunctiontype;
        runtime_fn_obj.fn = fn_ptr;
        reg[0] = X_MORPHO_OBJECT(&runtime_fn_obj);
    }
    reg_stat[0] = MORPHO_OBJECT(globalfn);

    // loading args into regs
    const size_t total_args = globalfn->nargs + globalfn->nopt;
    for (static_var<size_t> i = 0; i < total_args; i++) {
        reg[i + 1] = args[i + 1];
    }

    while (pc < n) {
        const instruction bc = instructions[pc];
        const int32_t a = DECODE_A(bc), b = DECODE_B(bc), c = DECODE_C(bc), bx = DECODE_Bx(bc), sbx = DECODE_sBx(bc);
        switch (DECODE_OP(bc)) {
            case OP_NOP:
                break;

            case OP_MOV:
                reg[a] = reg[b];
                reg_stat[a] = reg_stat[b];
                break;

            case OP_LCT:
                if (MORPHO_ISFUNCTION(globalfn->konst.data[bx])) {
                    // The purpose of the below block is to produce
                    // the C code that looks like:
                    //
                    //      struct userfn_object var1;
                    //      var1.type = 4;
                    //      var1.fn = morpho_userfn_f;
                    //      unsigned long int var2 = &var1;
                    //      unsigned long int var3 = MORPHO_OBJECT(var2)

                    const objectfunction *const morpho_fn = MORPHO_GETFUNCTION(globalfn->konst.data[bx]);

                    // this is less solid than I thought: name can be the emptystring, so there could be naming conflicts here
                    dyn_var<userfn *> fn_ptr = builder::with_name(get_mangled_fn_name(morpho_fn));
                    dyn_var<struct userfn_object> runtime_fn_obj;
                    runtime_fn_obj.type = objectfunctiontype;
                    runtime_fn_obj.fn = fn_ptr;
                    reg[a] = X_MORPHO_OBJECT(&runtime_fn_obj);
                    reg_stat[a] = globalfn->konst.data[bx];
                } else {
                    // for other objects...
                    // maybe generate C code equivalent to "get it from the RUNTIME
                    // constant table!" I think that's perfect
                    // aside from any objects types want to statically eval
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
                // // runtime::print(reg[a]);          // print morpho_fn at runtime
                // // object_print(NULL, reg_stat[a]); // print morpho_fn at PE time
    
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
                {
                    const value func = reg_stat[a];
                // if (MORPHO_ISMETAFUNCTION(func)) {
                //     metafunction_resolve(MORPHO_GETMETAFUNCTION(func), nargs, args + 1, NULL, &func);
                // }

                    if (MORPHO_ISFUNCTION(reg_stat[a])) {
                        dyn_var<userfn *> fn_ptr = builder::with_name(get_mangled_fn_name(MORPHO_GETFUNCTION(func)));
                        reg[a] = fn_ptr(&reg[a]);
                    }
                    else {
                        reg[a]=runtime::call(left, b, reg + a);
                    }
                }
                break;
            case OP_RETURN:
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
                throw UnimplementedInstructionException();
        }

        pc++;
    }
    throw OutOfBoundsPCException();
}

dyn_var<value> morpho_vm(
    dyn_var<value *> args,
    const int n,
    const instruction *const instructions,
    const objectfunction *const globalfn
) {
    // std::cerr << "toplevel\n";
    // declaration of globals is handled in header.c, no other way to make it
    // actually global, apparently
    return morpho_vm_rec(
        args,
        n,
        instructions,
        globalfn
    );
}