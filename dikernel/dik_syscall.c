#include <linux/vmalloc.h>
#include <linux/slab.h>     // for kmalloc
#include <linux/kmod.h>     // for request_module
#include <linux/dik/set_wrap.h> // for call_switcher_to_mod
#include <linux/dik/stack.h>    // for read_sp
#include <linux/dik/dacr.h>           // for read and write dacr
#include <linux/dik/myprint.h>
#include "table_walk.h"     // for read_ttbr

/***************** Kernel Thread Test *****************/

#include <linux/kthread.h>
#include <linux/dik/thread.h>
#include <linux/delay.h>        // for sleep
int foo_kthread(void * data) {
    char * str = (char*) data;
    dbg_pr("foo_kthread: I hope this data was really a string: %s.\n", 
            str);

    dbg_pr("read_DACR in thread: ");
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

/********************** DACR PoC ***********************/
#define ALLOC_SIZE (1 << 18)
#define DOMAIN 8
int * pt_dacr = NULL;
int * pt_dummy = NULL;
int * pt_corrupt = NULL;
void dacr_poc(unsigned domain_right) {
    int i;
    size_t new_dacr, current_dacr;

    if(!pt_dacr || !pt_dummy || !pt_corrupt) {
        printk("Allocation failed.\n");
        return;
    }

    corrupt_pt((unsigned int) pt_corrupt);
    corrupt_pt((unsigned int) pt_corrupt + ALLOC_SIZE);
    // from dikernel/table_walk.c function
    change_domain_id((unsigned int) pt_dacr, DOMAIN, ALLOC_SIZE);

    new_dacr = domain_val(DOMAIN, domain_right) |
        domain_val(DOMAIN_USER, DOMAIN_MANAGER) |
        domain_val(DOMAIN_KERNEL, DOMAIN_MANAGER) | 
        domain_val(DOMAIN_IO, DOMAIN_CLIENT);

    write_dacr(new_dacr);
    isb();
    read_dacr(current_dacr);
    printk("current dacr: 0x%x.\n", current_dacr);

    msleep(5000);
    for(i=1; i<ALLOC_SIZE/sizeof(int); i=i+10000)
        pt_dacr[i] = 42;
    printk("DACR didn't trigger bug, seeing if corruption does.\n");
    for(i=1; i<ALLOC_SIZE/sizeof(int); i=i+10000)
        pt_corrupt[i] = 42;
}

/******************** System Call *********************/
#include <asm/domain.h>
asmlinkage long sys_dikcall(void) {
    if(!pt_corrupt)
        pt_corrupt = kmalloc(ALLOC_SIZE, GFP_KERNEL);
    if(!pt_dummy)   // dummy so that pt_corrupt pt_dacr don't have common section
        pt_dummy = kmalloc(ALLOC_SIZE, GFP_KERNEL);
    if(!pt_dacr)
        pt_dacr = kmalloc(ALLOC_SIZE, GFP_KERNEL);
    //printk("kmallocing and putting it in an open domain\n");
    dacr_poc(DOMAIN_NOACCESS);
    //printk("kmallocing and putting it in a close domain\n");
    //dacr_poc(DOMAIN_NOACCESS);
    //printk("\ntest end.\n-----------------\n\n");


    //dump();   // Dump page tables
    return 0;
}
