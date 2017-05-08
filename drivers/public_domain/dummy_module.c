#include <linux/module.h>
#include <linux/dik/myprint.h>

MODULE_LICENSE("GPL");

#include <linux/dik/stack.h>
static int dummy_init(void) {
    dbg_pr("Hi everyone! I'm dummy module starting :-).\n");
    print_sp();
    return 0;
}

static void dummy_exit(void) {
    dbg_pr("Bye everyone. Dummy module is leaving :(.\n");
}

module_init(dummy_init);
module_exit(dummy_exit);
