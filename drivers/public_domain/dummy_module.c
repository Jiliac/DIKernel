#include <linux/module.h>

static int dummy_init(void) {
    printk("Hi everyone! I'm dummy module starting :-).\n");
    return 0;
}

static void dummy_exit(void) {
    printk("Bye everyone. Dummy module is leaving :(.\n");
}

module_init(dummy_init);
module_exit(dummy_exit);
