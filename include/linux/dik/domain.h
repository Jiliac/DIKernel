#include <linux/module.h>

#define DOMAIN_PUBLIC           3
#define DOMAIN_EXTENSION        4
#define DOMAIN_KERNEL_VIRTUAL   5

void module_change_domain(struct module *mod);
