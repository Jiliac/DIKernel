#ifndef _DIKSYMS_MODIF_H
#define _DIKSYMS_MODIF_H

#include <linux/module.h>

void modif_symbol(void);
Elf32_Sym * find_elf_syms(char *, struct module *);

#endif
