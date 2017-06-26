#include <linux/module.h>
#include <linux/dik/myprint.h>

MODULE_LICENSE("GPL");

static int dummy_init(void) {
    // Just to have code and stack usage.
    int i, j;
    i = 3;
    j = i+1;
    i = j;

    dbg_pr("Hello from dummy_init.\n");

    if(!j)
        return j;
    else
        return 0;
}

static void dummy_exit(void) {
    //dbg_pr("Bye everyone. Dummy module is leaving :(.\n");
}

module_init(dummy_init);
module_exit(dummy_exit);
