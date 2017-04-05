#include <linux/sched.h>    // for task_thread_info
#include <linux/dik/thread.h>

#include "dacr.h"           // for compute_DACR

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
    printk("Reading cpu_domain of a thread: %x.\n", 
        info->cpu_domain);
}

void read_task_ids(struct task_struct *task) {
    pid_t pid, tgid;
    pid = task->pid;
    tgid = task->tgid;
    printk("Task pid: %d - Task tgid: %d.\n", pid, tgid);
}
EXPORT_SYMBOL(read_task_ids);

void read_current_task_ids(void) {
    struct task_struct *task;
    task = current_thread_info()->task;
    read_task_ids(task);
}
EXPORT_SYMBOL(read_current_task_ids);
