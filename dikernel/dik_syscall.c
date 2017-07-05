#include <linux/vmalloc.h>
#include <linux/slab.h>     // for kmalloc
#include <linux/kmod.h>
#include <linux/dik/myprint.h>
#include <linux/dik/cyclecount.h>
#include <linux/dik/set_wrap.h>

#include <linux/timekeeping.h>
#define BIL     1000000000L

void look_time(void) {
    struct timespec ts;
    long times;
    long timen;

    do_posix_clock_monotonic_gettime(&ts);
    times = ts.tv_sec;
    timen = ts.tv_nsec;
    dbg_pr("time = %ld.%ld.\n", times, timen);
}

/******************** System Call *********************/
asmlinkage long sys_dikcall(void) {
    unsigned int t1, t2;

    if(1) {
        look_time();
        printk("toto\n");
        look_time();
    }

    if (0) {
        init_perfcounters();
        get_cyclecount(t1);
        printk("counting cycles...\n");
        get_cyclecount(t2);
        printk("delta:%d, t2:%d, t1:%d\n", t2-t1, t2, t1);
    }

    return 0;
}
