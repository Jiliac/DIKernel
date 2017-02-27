#include<linux/dik/wrapper.h>
#include "syms_modif.h"

void setting_wrappers() {
    const struct kernel_symbol * sym;
    const char * looking_for = "wrapper__kmalloc";
    printk("!!!!!!!!!! setting_wrappers start !!!!!!!!!!\n");

    sym = find_symbol(looking_for, NULL, NULL, true, true);
    if(sym)
        modify_symbol("__kmalloc", sym->value);
    else
        printk("dik/wrapper.c:setting_wrappers couldn't find %s symbol to "
            "modify.\n", looking_for);
    printk("!!!!!!!!!! setting_wrappers end !!!!!!!!!!\n");
}

void * wrapper__kmalloc(size_t size, gfp_t gfp) {
    printk("Calling __kmalloc, but through a wrapper.\n");
    return __kmalloc(size, gfp);
}
EXPORT_SYMBOL(wrapper__kmalloc);
