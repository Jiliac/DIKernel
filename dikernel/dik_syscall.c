#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>     // for kmalloc
#include <linux/kmod.h>     // for request_module
#include <linux/dik/set_wrap.h> // for call_switcher_to_mod
#include <linux/dik/stack.h>    // for read_sp
#include <linux/dik/dacr.h>           // for read and write dacr
#include "table_walk.h"     // for read_ttbr

#define VMALLOC_SIZE   1000000 
#define BUF_SIZE       1000000 

/***************** Kernel Thread Test *****************/

#include <linux/kthread.h>
#include <linux/dik/thread.h>
#include <linux/delay.h>        // for sleep
int foo_kthread(void * data) {
    char * str = (char*) data;
    printk("foo_kthread: I hope this data was really a string: %s.\n", 
        str);

    printk("read_DACR in thread: ");
    read_DACR();

    do_exit(0);
    return 0;
}

void kthread_run_test(void) {
    struct task_struct *task;
    task = kthread_create(foo_kthread, "toto string", "KThread test!");

    modify_task_DACR(4, 1, task);

    wake_up_process(task);
    read_thread_cpu_domain(task);
    msleep(1000);
    read_thread_cpu_domain(task);
}

/******************** System Call *********************/
asmlinkage long sys_dikcall(void) {
    print_sp();
    read_DACR();
    write_cpu_domain(4, 1);
    read_DACR();

    //kthread_run_test();

    return 0;
}
