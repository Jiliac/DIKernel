#include <linux/sched.h>    // for task_thread_info
#include <linux/dik/thread.h>
#include <linux/dik/dacr.h>           // for compute_DACR
#include "table_walk.h"     // change_domain_id
#include <asm/tlbflush.h>
#include <asm/page.h>       // also for flush
#include <linux/dik/myprint.h>

// This is for debug. Likely to be removed later
void change_stack_back(size_t domain_id, unsigned int stack) {
    if(domain_id < 16) {
        change_domain_id(stack, domain_id);
        flush_tlb_kernel_page(stack);
        flush_tlb_kernel_page(stack + PAGE_SIZE);
    } else {
        // alternative mode to walk page tables
        unsigned int* section_base_addr;
        unsigned int section_base;
        section_base_addr = get_first_lvl(stack);
        section_base = *section_base_addr;
        dbg_pr("Current domain of 0x%8x section is %i.\n", section_base,
            get_domain_id(section_base));
    }
}
EXPORT_SYMBOL(change_stack_back);
/***************** debug end *********************/

unsigned long set_task_stack_domain_id(size_t domain_id, struct task_struct *task) {
    unsigned int stack = (unsigned int) task->stack;
    change_domain_id(stack, domain_id);
    flush_tlb_kernel_page(stack);
    flush_tlb_kernel_page(stack + PAGE_SIZE);
    return stack;
}
EXPORT_SYMBOL(set_task_stack_domain_id);

void set_task_DACR(size_t new_dacr, struct task_struct *task) {
    struct thread_info *info;
    info = task_thread_info(task);
    info->cpu_domain = new_dacr;
}
EXPORT_SYMBOL(set_task_DACR);

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
    dbg_pr("Reading cpu_domain of a thread: %x.\n", 
        info->cpu_domain);
}
EXPORT_SYMBOL(read_thread_cpu_domain);

void read_current_thread_cpu_domain(void) {
    struct task_struct *task;
    task = current_thread_info()->task;
    read_thread_cpu_domain(task);
}
EXPORT_SYMBOL(read_current_thread_cpu_domain);

void read_task_ids(struct task_struct *task) {
    pid_t pid, tgid;
    pid = task->pid;
    tgid = task->tgid;
    dbg_pr("Task pid: %d - Task tgid: %d.\n", pid, tgid);
}
EXPORT_SYMBOL(read_task_ids);

void read_current_task_ids(void) {
    struct task_struct *task;
    task = current_thread_info()->task;
    read_task_ids(task);
}
EXPORT_SYMBOL(read_current_task_ids);
