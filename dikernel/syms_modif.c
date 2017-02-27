//#include <linux/kallsyms.h>
#include <linux/elf.h>
#include "syms_modif.h"

#define MODULE_SEARCH   "tuner_xc2028"
#define SYM_SEARCH      "xc2028_set_config"

/****************************************************************************
********************************** Utility **********************************
****************************************************************************/
#ifdef CONFIG_KALLSYMS

void modify_elf_sym(Elf_Sym * sym, unsigned long value) {
    sym->st_value = value;
}

Elf_Sym * find_elf_sym(char * name, struct module * mod) {
    Elf_Sym * start;
    int i;
    char * sym_name;

    start = mod->core_symtab;
    for(i = 0; i < mod->num_symtab; i++) {
        sym_name = mod->strtab + start[i].st_name;
        printk("dikernel/syms_modif.c: dumping elf nb %i - "
            "value: %x - name: %s.\n", i,
            start[i].st_value, sym_name);
        if(!strcmp(name, sym_name))
            return start + i;
    }
    return NULL;
}

#endif /*CONFIG_KALLSYMS*/

/****************************************************************************
*************************** General Sym Modification ************************
****************************************************************************/

void modify_symbol(char * target_name, unsigned long new_value) {
    struct kernel_symbol * sym;
    
    sym = (struct kernel_symbol*) find_symbol(target_name, NULL, NULL, true, true);
    if(sym){
        printk("dikcmd/syms_modif.c: We found %s symbol!\n", target_name);
    }
    else{
        printk("dikcmd/syms_modif.c: didn't find %s symbol.\n", target_name);
        return;
    }
    sym->value = new_value;
}

/****************************************************************************
*************************** add_kallsyms analysis ***************************
****************************************************************************/
static const struct kernel_symbol * find_dummy_sym(void);

static void print_symbol(const struct kernel_symbol * sym) {
    printk("%s value: %lx\n", sym->name, sym->value);
}

void modif_symbol() {
    struct module * mod = NULL;
    struct kernel_symbol * sym;

    mod = find_module(MODULE_SEARCH);
    if(!mod) {
        printk("dikcmd/syms_modif.c: didn't find the module.\n");
        return;
    } else 
        printk("dikcmd/syms_modif.c: found %s.\n", mod->name);
    sym = (struct kernel_symbol*) find_symbol(SYM_SEARCH, &mod, NULL, true, true);

    if(sym) {
        printk("dikcmd/syms_modif.c: We found a kernel symbol!\n");
        print_symbol(sym);
        // Modifying the "effective" value of the symbol
        // (the one used by the kernel)
        sym->value = find_dummy_sym()->value;
        print_symbol(sym);

#ifdef CONFIG_KALLSYMS
        // Now modifying the /proc/kallsyms value for users.
        modify_elf_sym(find_elf_sym(SYM_SEARCH, mod),
            find_dummy_sym()->value);
#endif /*CONFIG_KALLSYMS*/

    } else {
        const struct kernel_symbol * start;
        int i;
        printk("dikcmd/syms_modif.c: Failed to find a kernel symbol.\n");
        start = mod->syms;
        for(i= 0; i < mod->num_syms; i++) {
            print_symbol(start+i);
        }
    }

    return;
}

/****************************************************************************
******************* Providing a dummy symbol to point to ********************
****************************************************************************/

void dummy_symbol(void) {
    printk("You are pointing to a dummy symbol :p .\n");
}
EXPORT_SYMBOL(dummy_symbol);

static const struct kernel_symbol * find_dummy_sym() {
    const struct kernel_symbol * sym;
    
    sym = find_symbol("dummy_symbol", NULL, NULL, true, true);
    if(sym){
        printk("dikcmd/syms_modif.c: We found a kernel symbol!\n");
        return sym;
    }
    else
        printk("dikcmd/syms_modif.c: didn't find a kernel symbol :(\n");

    return sym;
}
