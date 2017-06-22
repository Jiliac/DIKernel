#include <linux/elf.h>
#include "syms_modif.h"
#include <linux/dik/myprint.h>

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
        dbg_pr("dikernel/syms_modif.c: dumping elf nb %i - "
            "value: %x - name: %s.\n", i,
            start[i].st_value, sym_name);
        if(!strcmp(name, sym_name))
            return start + i;
    }
    return NULL;
}

#endif /*CONFIG_KALLSYMS*/

/****************************************************************************
*************************** Code Symbol Modification ************************
****************************************************************************/

void modify_symbol(char * target_name, unsigned long new_value) {
    struct kernel_symbol * sym;

    sym = (struct kernel_symbol*) find_symbol(target_name, NULL, NULL,
            true, true);
    if(sym){
        dbg_pr("dikcmd/syms_modif.c: We found %s symbol!\n",
                target_name);
    }
    else{
        dbg_pr("dikcmd/syms_modif.c: didn't find %s symbol.\n",
                target_name);
        return;
    }
    sym->value = new_value;
}
