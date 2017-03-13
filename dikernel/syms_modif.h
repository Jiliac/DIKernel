#ifndef _DIKSYMS_MODIF_H
#define _DIKSYMS_MODIF_H

#include <linux/module.h>

void modify_symbol(char * target_name, unsigned long new_value);
Elf32_Sym * find_elf_syms(char *, struct module *);

#endif
