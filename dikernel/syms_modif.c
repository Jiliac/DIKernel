//#include <linux/kallsyms.h>
#include <linux/module.h>   // for find_module
#include "syms_modif.h"

#define MODULE_SEARCH   "tuner_xc2028"
#define SYM_SEARCH      "xc2028_set_config"

void print_symbol(const struct kernel_symbol * sym) {
    printk("%s value: %lx\n", sym->name, sym->value);
}

void modif_symbol() {
    struct module * mod = NULL;
    struct kernel_symbol * sym;

    mod = find_module(MODULE_SEARCH);
    if(!mod) {
        printk("dikcmd/kallsyms_modif.c: didn't find the module.\n");
        return;
    } else 
        printk("dikcmd/kallsyms_modif.c: found %s.\n", mod->name);
    sym = (struct kernel_symbol*) find_symbol(SYM_SEARCH, &mod, NULL, true, true);

    if(sym) {
        printk("dikcmd/kallsyms_modif.c: We found a kernel symbol!\n");
        print_symbol(sym);
        sym->value = sym->value + 8;
        print_symbol(sym);
    } else {
        const struct kernel_symbol * start;
        int i;
        printk("dikcmd/kallsyms_modif.c: Failed to find a kernel symbol.\n");
        start = mod->syms;
        for(i= 0; i < mod->num_syms; i++) {
            print_symbol(start+i);
        }
    }

    return;
}
