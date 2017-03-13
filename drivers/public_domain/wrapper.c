#include <linux/module.h>
#include<linux/slab.h>

MODULE_LICENSE("GPL");

/************ initcall wrapper ************/

#include<linux/init.h>
int call_initfunc(initcall_t fn) {
    printk("drivers/public_domain/wrapper.c: Just calling the init function.\n");
    return fn();
}
//EXPORT_SYMBOL(call_initfunc); // not needed right?

/************ exitcall wrapper ************/

void call_exitfunc(void (*fn) (void)) {
    printk("drivers/public_domain/wrapper.c: Just calling the exit "
        "function.\n");
    fn();
}

/****************** kmalloc wrapper ******************/

void * wrapper__kmalloc(size_t size, gfp_t gfp) {
    printk("Calling __kmalloc, but through a wrapper.\n");
    return __kmalloc(size, gfp);
}
EXPORT_SYMBOL(wrapper__kmalloc);

/************ standard module stuff ******************/

extern void register_wrapper_initcall(void * ptr);
extern void register_wrapper_exitcall(void * ptr);
void register_wrappers(void) {
    register_wrapper_initcall(call_initfunc);
    register_wrapper_exitcall(call_exitfunc);
}

static int wrapper_init(void) {
    printk("drivers/public_domain/wrapper.c: init - registering wrappers.\n");
    register_wrappers();
    return 0;
}

module_init(wrapper_init);
