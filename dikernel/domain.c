#include <linux/dik/domain.h>
#include <linux/dik/set_wrap.h>  // get_wrapper_set
#include <linux/dik/myprint.h>
#include "table_walk.h"

void module_change_domain(struct module *mod) {
    // Check if we are passed booting time.
    if(!get_wrapper_set())
        return;

    change_domain_id((unsigned int) mod->module_core, DOMAIN_EXTENSION);
    if(mod->module_init) {
        change_domain_id((unsigned int) mod->module_init, DOMAIN_EXTENSION);
    }
}
