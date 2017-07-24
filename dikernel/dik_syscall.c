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
    return 0;
}
