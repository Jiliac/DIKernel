#include <linux/dik/domain.h>
#include "table_walk.h"

void module_change_domain(struct module *mod) {
    modify_domain_id((unsigned long int) mod->module_core, 0);
    if(mod->module_init) {
        modify_domain_id((unsigned long int) mod->module_init, 0);
    }
}
