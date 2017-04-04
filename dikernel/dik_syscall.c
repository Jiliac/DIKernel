#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>     // for kmalloc
#include <linux/kmod.h>     // for request_module
#include <linux/dik/set_wrap.h> // for call_switcher_to_mod
#include <linux/dik/stack.h>    // for read_sp
#include <linux/sched.h>    // for task_thread_info
#include "dacr.h"           // for read and write dacr
#include "table_walk.h"     // for read_ttbr

#define VMALLOC_SIZE   1000000 
#define BUF_SIZE       1000000 

/***************** Kernel Thread Test *****************/

#include <linux/kthread.h>

// two following function should be moved in another file.
void modify_task_DACR(size_t domain, size_t type, struct task_struct *task) {
    size_t new_dacr;
    struct thread_info *info;
    info = task_thread_info(task);
    new_dacr = compute_DACR(domain, type, info);
    info->cpu_domain = new_dacr;
}

void read_thread_cpu_domain(struct task_struct *task) {
    struct thread_info *info;
    info = task_thread_info(task);
    printk("reading cpu_domain of a thread: %x.\n", 
        info->cpu_domain);
}

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

/******************************************************/

void kmallocing(void) {
    char *buf;
    int i;

    printk("kmalloc-inc to check its allocation process\n");
    buf = kmalloc(BUF_SIZE, GFP_ATOMIC);
    for(i = 0; i < BUF_SIZE / sizeof(char); i++) {
        buf[i] = 42;
    }
}

asmlinkage long sys_dikcall(void) {
    print_sp();
    read_DACR();
    write_DACR(4, 1);
    read_DACR();

    return 0;
}
