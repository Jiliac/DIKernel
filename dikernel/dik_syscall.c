#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>     // for kmalloc
#include <linux/kmod.h>     // for request_module
#include <linux/dik/set_wrap.h> // for call_switcher_to_mod
#include <linux/dik/stack.h>    // for read_sp
#include <linux/dik/dacr.h>           // for read and write dacr
#include <linux/dik/myprint.h>
#include "table_walk.h"     // for read_ttbr

#define BUF_SIZE       1000000 

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

/******************** System Call *********************/
#include <asm/domain.h>
#include <linux/dik/dacr.h>     // write_dacr and read_dacr macros
asmlinkage long sys_dikcall(void) {
    size_t old_dacr, new_dacr;

#ifdef CONFIG_DIK_EVA
    dbg_pr("dik_eva: %d\n", CONFIG_DIK_EVA);
#endif

    read_dacr(old_dacr);
    new_dacr = (old_dacr & (~domain_val(DOMAIN_USER, DOMAIN_MANAGER)))
        | domain_val(DOMAIN_USER, DOMAIN_NOACCESS);
    dbg_pr("old dacr: 0x%x, new_dacr: 0x%x.\n", old_dacr, new_dacr);
    write_dacr(new_dacr);

    dump();

    return 0;
}
