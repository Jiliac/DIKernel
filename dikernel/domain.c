#include <linux/dik/domain.h>
#include<linux/dik/set_wrap.h>  // get_wrapper_set
#include <linux/string.h>       // strcmp
#include "table_walk.h"

/* DOMAIN_KERNEL and DOMAIN_CLIENT reserved for system
 */
static size_t free_domains = (1 << DOMAIN_KERNEL) | (1 << DOMAIN_TABLE) |
    (1 << DOMAIN_USER) | (1 << DOMAIN_IO) | (1 << DOMAIN_PUBLIC) |
    (1 << DOMAIN_KERNEL_VIRTUAL);

size_t get_free_domain(void) {
    int i;
    int dom_availability;
    for(i = 4; i < 16; ++i) {
        dom_availability = (free_domains >> i) & 1;
        if(!dom_availability) {
            free_domains |= 1 << i;
            return i;
        }
    }
    printk("\n");
    // no domain available
    return -1;
}

void set_mod_domain(struct module *mod) {
    size_t *dom_id = &(mod->mod_domain);
    size_t free_dom;
    if(*dom_id > -1 && *dom_id < 16) 
        return; // already set

    if(!strcmp(mod->name, WRAPPER_MODULE)) {
        printk("%s.ko always get assigned DOMAIN_PUBLIC.\n", mod->name);
        *dom_id = DOMAIN_PUBLIC;
        return;
    }

    free_dom = get_free_domain();
    if(free_dom != -1)
        *dom_id = free_dom;
    else
        /* This should be a complexe domain management.
         * Now this is a security vulnerability easily exploitable
         * because an attacker just has to "overload" the available domain ids.
         */
        *dom_id = DOMAIN_KERNEL;
    printk("dikernel/domain.c:set_mod_domain module %s gets assigned "
        "domain %d.\n", mod->name, *dom_id);
}

void module_change_domain(struct module *mod) {
    // Check if we are passed booting time.
    if(!get_wrapper_set())
        return;

    set_mod_domain(mod);
    change_domain_id((unsigned int) mod->module_core, mod->mod_domain);
    if(mod->module_init) {
        change_domain_id((unsigned int) mod->module_init, mod->mod_domain);
    }
}
