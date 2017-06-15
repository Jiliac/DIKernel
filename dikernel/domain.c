#include <linux/dik/domain.h>
#include <linux/dik/dacr.h>
#include <linux/dik/set_wrap.h>  // get_wrapper_set
#include <linux/dik/myprint.h>
#include "table_walk.h"

void module_change_domain(struct module *mod) {
    // Check if we are passed booting time.
    if(!get_wrapper_set())
        return;

    dbg_pr("dikernel/domain.c: module_change_domain modifying extension code "
        "domain.\n");
    change_domain_id((unsigned int) mod->module_core, DOMAIN_EXTENSION,
        mod->core_size);
    if(mod->module_init) {
        change_domain_id((unsigned int) mod->module_init, DOMAIN_EXTENSION,
            mod->init_size);
    }
}

size_t addr_domain_id(unsigned int addr) {
    unsigned int *first_lvl_addr = get_first_lvl(addr);
    unsigned int first_lvl = *first_lvl_addr;
    size_t domain = get_domain_id(first_lvl);
    return domain;
}
EXPORT_SYMBOL(addr_domain_id);
