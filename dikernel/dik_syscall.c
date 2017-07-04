#include <linux/vmalloc.h>
#include <linux/slab.h>     // for kmalloc
#include <linux/kmod.h>
#include <linux/dik/myprint.h>
#include <linux/dik/cyclecount.h>

/******************** System Call *********************/
asmlinkage long sys_dikcall(void) {
    unsigned int t1, t2;
    init_perfcounters();
    get_cyclecount(t1);
    printk("counting cycles...\n");
    get_cyclecount(t2);
    printk("delta:%d, t2:%d, t1:%d\n", t2-t1, t2, t1);
    return 0;
}
