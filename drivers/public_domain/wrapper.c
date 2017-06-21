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

/****************** some symbol wrappers ******************/

#include <linux/slab.h>
void * wrapper___kmalloc(size_t size, gfp_t gfp) {
    void * ret;
    entry_gate(wrapper___kmalloc_label);
    dbg_pr("Calling __kmalloc, but through a wrapper.\n");
    ret = __kmalloc(size, gfp);
    dbg_pr("Called __kmalloc through a wrapper.\n");
    post_call();
    return ret;
wrapper___kmalloc_label:
    return wrapper___kmalloc(size, gfp);
}
EXPORT_SYMBOL(wrapper___kmalloc);

extern void __aeabi_unwind_cpp_pr1(void);
void wrapper___aeabi_unwind_cpp_pr1(void) {
    entry_gate(wrapper___aeabi_unwind_cpp_pr1_label);
    dbg_pr("Calling __aeabi_unwind_cpp_pr1 through a wrapper.\n");
    __aeabi_unwind_cpp_pr1();
    post_call();
    return;
wrapper___aeabi_unwind_cpp_pr1_label:
    wrapper___aeabi_unwind_cpp_pr1();
}
EXPORT_SYMBOL(wrapper___aeabi_unwind_cpp_pr1);


static int wrapper_init(void) {
    dbg_pr("drivers/public_domain/wrapper.c: init - registering wrappers.\n");
    return 0;
}

module_init(wrapper_init);
