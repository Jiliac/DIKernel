#include <linux/module.h>
#include <linux/dik/myprint.h>

MODULE_LICENSE("GPL");

// for evaluation
#include <linux/slab.h>
#include <linux/dik/cyclecount.h>
#define     LOOP_NB 200

static int dummy_init(void) {
    // Just to have code and stack usage.
    int i;
    int *ptr[LOOP_NB];
    unsigned int cc_before, cc_after;

    //dbg_pr("Hello from dummy_init.\n");

    for(i = 0; i<LOOP_NB; ++i) {
        get_cyclecount(cc_before);
        ptr[i] = __kmalloc(sizeof(int), GFP_KERNEL);
        get_cyclecount(cc_after);
        printk("wrapper_call kmalloc. cc_before: %i - cc_after: %i\n",
            cc_before, cc_after);
        *(ptr[i]) = 22;
        isb();
    }
    for (i = 0; i<LOOP_NB; ++i) {
        get_cyclecount(cc_before);
        kfree(ptr[i]);
        get_cyclecount(cc_after);
        printk("wrapper_call kfree. cc_before: %i - cc_after: %i\n",
            cc_before, cc_after);
        isb();
    }
    

    return 0;
}

static void dummy_exit(void) {
    //dbg_pr("Bye everyone. Dummy module is leaving :(.\n");
}

module_init(dummy_init);
module_exit(dummy_exit);
