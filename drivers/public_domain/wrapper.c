#include <linux/module.h>
#include <linux/dik/myprint.h>
#include <linux/dik/dacr.h>     // write_dacr and read_dacr macros
#include "gates.h"

MODULE_LICENSE("GPL");

/***************** factorizing wrapper code ***************/
static void post_call(void) {
    exit_gate();
    dbg_pr("After the exit gate, changing DACR to 0x30df to trigger bug.\n");
    write_dacr(0x30df);
}

/****************** data symbol wrappers ******************/

/****************** code symbol wrappers ******************/

#include <linux/slab.h>
void * wrapper__kmalloc(size_t size, gfp_t gfp) {
    void * ret;
    entry_gate(wrapper__kmalloc_label);
    dbg_pr("Calling __kmalloc, but through a wrapper.\n");
    ret = __kmalloc(size, gfp);
    dbg_pr("Called __kmalloc through a wrapper.\n");
    post_call();
    return ret;
wrapper__kmalloc_label:
    return wrapper__kmalloc(size, gfp);
}
EXPORT_SYMBOL(wrapper__kmalloc);

extern void __aeabi_unwind_cpp_pr1(void);
void wrapper__aeabi_unwind_cpp_pr1(void) {
    entry_gate(wrapper__aeabi_unwind_cpp_pr1_label);
    dbg_pr("Calling __aeabi_unwind_cpp_pr1 through a wrapper.\n");
    __aeabi_unwind_cpp_pr1();
    post_call();
    return;
wrapper__aeabi_unwind_cpp_pr1_label:
    wrapper__aeabi_unwind_cpp_pr1();
}
EXPORT_SYMBOL(wrapper__aeabi_unwind_cpp_pr1);


static int wrapper_init(void) {
    dbg_pr("drivers/public_domain/wrapper.c: init - registering wrappers.\n");
    return 0;
}

module_init(wrapper_init);
