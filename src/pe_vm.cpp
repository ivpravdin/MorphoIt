#include "pe_vm.h"

#include <exception>
#include "builder/dyn_var.h"
#include "builder/static_var.h"
#include "builder/lib/utils.h"

#include "value.h"
#include "runtime.h"
#include "pe_vm_consts.h"


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
    const objectfunction * const globalfn,
    std::map<uintptr_t, block::stmt::Ptr> &subfn_asts
) {
    // declare arr "reg" with nregs elements
    dyn_var<value[]> reg = builder::with_name("reg", true);
    builder::resize_arr(reg, globalfn->nregs);

    dyn_var<value[PE_NUM_GLOBALS]> globals = builder::with_name(PE_GLOBALSBUF_NAME);
    dyn_var<value> left = builder::with_name("left", true), right = builder::with_name("right", true);

    std::vector<static_var<value>> reg_stat(globalfn->nregs);
    std::vector<static_var<pe_t_note>> reg_type(globalfn->nregs);
    std::vector<static_var<pe_t_note>> global_type(PE_NUM_GLOBALS);

    for (static_var<size_t> i = 0; i < globalfn->nregs; i++) {
        reg_stat[i] = MORPHO_NIL;
        reg_type[i] = pe_t_note::UNKNOWN;
    }

    for (static_var<size_t> i = 0; i < PE_NUM_GLOBALS; i++) {
        global_type[i] = pe_t_note::UNKNOWN;
    }

    static_var<instruction> pc = globalfn->entry;

    // init'ing reg[0]
    // for now we'll just do this, since, e.g. the globalfn has a NULL name
    if (MORPHO_ISOBJECT(globalfn->name)) {
        dyn_var<struct userfn_object> runtime_fn_obj = builder::with_name(get_mangled_fnobj_name(globalfn));
        reg[0] = X_MORPHO_OBJECT(&runtime_fn_obj);
    }
    reg_stat[0] = MORPHO_OBJECT(globalfn);
    reg_type[0] = pe_t_note::OBJECT;

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
                reg_type[a] = reg_type[b];
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
                    dyn_var<struct userfn_object> runtime_fn_obj = builder::with_name(get_mangled_fnobj_name(morpho_fn));;
                    reg[a] = X_MORPHO_OBJECT(&runtime_fn_obj);
                    reg_stat[a] = globalfn->konst.data[bx];
                } else {
                    // for other objects...
                    // maybe generate C code equivalent to "get it from the RUNTIME
                    // constant table!" I think that's perfect
                    // aside from any objects types want to statically eval
                    reg[a] = globalfn->konst.data[bx];
                }
                reg_type[a] = gettypeannotation(globalfn->konst.data[bx]);
                // std::cerr << "Loaded const of type: " << reg_type[a] << "\n";
                break;

            case OP_ADD:
                left = reg[b], right = reg[c];
                reg[a] = op_add(left, right, reg_type[b], reg_type[c]);

                reg_type[a] = arith_binop_typerule(reg_type[b], reg_type[c]);
                break;

            case OP_SUB:
                left = reg[b], right = reg[c];

                reg[a] = runtime::op_sub(left, right);
                reg_type[a] = arith_binop_typerule(reg_type[b], reg_type[c]);
                break;

            case OP_MUL:
                left = reg[b], right = reg[c];
                reg[a] = runtime::op_mul(left, right);
                reg_type[a] = arith_binop_typerule(reg_type[b], reg_type[c]);

                break;

            case OP_DIV:
                left = reg[b], right = reg[c];
                reg[a] = runtime::op_div(left, right);

                reg_type[a] = pe_t_note::FLOAT;
                break;

            case OP_POW: //POW
                left = reg[b];
                right = reg[c];
                reg[a] = runtime::op_pow(left, right);

                reg_type[a] = pe_t_note::FLOAT;
                break;

            case OP_EQ:
                left = reg[b];
                right = reg[c];

                // reg[a] = X_MORPHO_BOOL(!x_morpho_extendedcomparevalue(left, right, reg_type[b], reg_type[c]));
                reg[a] = X_MORPHO_BOOL(!x_morpho_extendedcomparevalue(left, right, reg_type[b], reg_type[c]));
                reg_type[a] = pe_t_note::BOOL;
                break;
            case OP_NEQ:
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(x_morpho_extendedcomparevalue(left, right, reg_type[b], reg_type[c]));
                reg_type[a] = pe_t_note::BOOL;
                break;
            case OP_NOT:
                left = reg[b];

                reg[a] = runtime::op_not(left);
                reg_type[a] = pe_t_note::BOOL;
                break;
            case OP_LT: //LT
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(x_morpho_extendedcomparevalue(left, right, reg_type[b], reg_type[c]) > MORPHO_EQUAL);
                reg_type[a] = pe_t_note::BOOL;
                break;
            case OP_LE: //LT
                left = reg[b];
                right = reg[c];

                reg[a] = X_MORPHO_BOOL(x_morpho_extendedcomparevalue(left, right, reg_type[b], reg_type[c]) >= MORPHO_EQUAL);
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
                {
                    // global state is opaque after a function call
                    for (static_var<size_t> i = 0; i < PE_NUM_GLOBALS; i++) {
                        global_type[i] = pe_t_note::UNKNOWN;
                    }

                    const value func = reg_stat[a];

                    if (MORPHO_ISFUNCTION(reg_stat[a])) {
                        dyn_var<userfn *> fn_ptr = builder::with_name(get_mangled_fn_name(MORPHO_GETFUNCTION(func)));
                        reg[a] = fn_ptr(&reg[a]);
                        // reg_type[a] = MORPHO_GETFUNCTION(func)->sig.ret;
                    }
                    else {
                        reg[a]=runtime::call(left, b, reg + a);
                        // reg_type[a] = TAG_UNDEF_T;
                    }
                }
                // for now function calls will be completely opaque
                reg_type[a] = pe_t_note::UNKNOWN;
                break;
            case OP_RETURN:
                if (a>0)
                    return reg[b];
                else
                    return MORPHO_NIL;
                break;
            case OP_LGL: // LGL
                reg[a]      = globals[bx];
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

dyn_var<value> morpho_vm(
    dyn_var<value *> args,
    const int n,
    const instruction *const instructions,
    const objectfunction *const globalfn,
    std::map<uintptr_t, block::stmt::Ptr> &subfn_asts
) {
    // std::cerr << "toplevel\n";
    // declaration of globals is handled in header.c, no other way to make it
    // actually global, apparently
    return morpho_vm_rec(
        args,
        n,
        instructions,
        globalfn,
        subfn_asts
    );
}