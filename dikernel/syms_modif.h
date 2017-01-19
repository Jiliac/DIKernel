#ifndef _DIKSYMS_MODIF_H
#define _DIKSYMS_MODIF_H

#include <linux/module.h>

void modif_symbol(void);
const struct kernel_symbol * find_dummy_sym(void);

#endif
