#include <linux/module.h>

#define DOMAIN_PUBLIC           3
#define DOMAIN_EXTENSION        4
#define DOMAIN_KERNEL_VIRTUAL   5

void module_change_domain(struct module *mod);
extern size_t addr_domain_id(unsigned int addr);
