#include <linux/module.h>

#define DOMAIN_EXTENSION        3

void module_change_domain(struct module *mod);
extern size_t addr_domain_id(unsigned int addr);
extern void change_domain_ext_ptr(void * ptr, unsigned int size);
