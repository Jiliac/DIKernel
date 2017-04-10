#include <linux/module.h>

#define DOMAIN_PUBLIC   3

void module_change_domain(struct module *mod);
