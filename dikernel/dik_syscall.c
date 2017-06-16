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
#include <linux/dik/thread.h>   // read_current_task_ids
#define walk_registers()                                        \
    do {                                                        \
        size_t reg;                                             \
        /*** for stack pointer ***/                             \
        asm volatile("mov %0, r13" : "=r" (reg) :);             \
        dbg_pr("Current Stack Pointer (SP): 0x%8x.\n", reg);    \
        change_stack_back(20, reg);                             \
                                                                \
        /*** for programm counter ***/                          \
        asm volatile("mov %0, r15" : "=r" (reg) :);             \
        dbg_pr("Current Program Counter (PC): 0x%8x.\n", reg);  \
        change_stack_back(20, reg);                             \
    } while(0)

#define ALLOC_SIZE (1 << 18)
#define DOMAIN 8
int * pt_dacr = NULL;
void dacr_poc(unsigned domain_right) {
    int i;
    size_t new_dacr, old_dacr;

    if(!pt_dacr) {
        printk("Allocation failed.\n");
        return;
    }

    change_domain_id((unsigned int) pt_dacr, DOMAIN, ALLOC_SIZE);

    new_dacr = domain_val(DOMAIN, domain_right) |
        domain_val(DOMAIN_USER, DOMAIN_MANAGER) |
        domain_val(DOMAIN_KERNEL, DOMAIN_MANAGER) | 
        domain_val(DOMAIN_IO, DOMAIN_CLIENT)    |
        domain_val(12,  DOMAIN_CLIENT);

    walk_registers();
    
    read_dacr(old_dacr);
    printk("current dacr: 0x%x, new one: 0x%x.\n", old_dacr, new_dacr);
    write_dacr(new_dacr);
    asm volatile("push {r0}\n\tpop {r0}");
    write_dacr(old_dacr);
    printk("Closed domain %d and opened it again.\n", DOMAIN);
    write_dacr(new_dacr);
    isb();

    for(i=1; i<ALLOC_SIZE/sizeof(int); i=i+10000)
        pt_dacr[i] = 42;
    write_dacr(old_dacr);
    printk("Reached the end of dacr_poc?\n");
}

/******************** System Call *********************/
#include <asm/domain.h>
asmlinkage long sys_dikcall(void) {
    if(!pt_dacr)
        pt_dacr = vmalloc(ALLOC_SIZE/*, GFP_KERNEL*/);
        // use vmalloc because kmalloc allocates just next to the used stack.
    dacr_poc(DOMAIN_NOACCESS);

    return 0;
}
