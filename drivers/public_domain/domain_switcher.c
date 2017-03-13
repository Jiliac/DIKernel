#include <linux/module.h>

MODULE_LICENSE("GPL");

/***************** switch from kernel to mod **************/
/* Should be called from kernel */

void switch_to_basekernel(void) {
    printk("Switching from module to base kernel.\n");
}
EXPORT_SYMBOL(switch_to_basekernel);

/****************** switch from mod to kernel *************/

void switch_to_module(void) {
    printk("Switching from base kernel to module.\n");
}
EXPORT_SYMBOL(switch_to_module);

extern void register_switcher_to_mod(void * ptr);
void register_switcher(void) {
    register_switcher_to_mod(switch_to_module);
}

/********** registering our function in base kernel *******/

static int public_init(void) {
    printk("domain_switcher module_init registering switch_to_module "
        "function.\n");
    register_switcher();
    return 0;
}

static void public_exit(void) {
    printk("domain_switcher module_exit also has nothing to do.\n");
}

module_init(public_init);
module_exit(public_exit);
