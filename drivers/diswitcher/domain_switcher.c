#include <linux/module.h>
#include <linux/kthread.h>  // for kthreads
#include <linux/dik/myprint.h>
#include "gates.h"

MODULE_LICENSE("GPL");

/************************************************************************/
/* Factorizing as much code as possible  for exit and init call wrapper */
/************************************************************************/
#include <linux/dik/stack.h>    // print stack pointer
#include <linux/dik/thread.h>   // read_current_task_ids
#include <linux/dik/domain.h>   // DOMAIN_EXTENSION
#include <linux/dik/dacr.h>     // write_dacr and read_dacr macros
#include <asm/thread_info.h>    // for the macros (current_thread_info)

/******* Debug Function *********/
#ifdef DEBUG
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
extern void
change_domain_id(unsigned int addr, size_t domain_id, unsigned int size);
static void change_reg_ids(void) {
    size_t sp, pc;
    asm volatile("mov %0, r13" : "=r" (sp) :);
    asm volatile("mov %0, r15" : "=r" (pc) :);
    change_domain_id((unsigned int) sp, DOMAIN_EXTENSION, 0);
    change_domain_id((unsigned int) pc, DOMAIN_EXTENSION, 0);
}
static void open_close(unsigned n) {
    write_dacr(0x5555555f);
    dbg_pr("We are here at bug point %d.\n", n);
    write_dacr(EXIT_DACR);
}
#else
#define walk_registers()
static void change_reg_ids(void) {}
static void open_close(unsigned n) {}
#endif
/********************************/

struct sync_args {
#ifdef CONFIG_DIK_USE_THREAD
    wait_queue_head_t *wq;
    int *event;
#endif
};

static inline void wake_thread(struct sync_args *sync, unsigned int args_addr) {
#ifdef CONFIG_DIK_USE_THREAD
    size_t domain = addr_domain_id(args_addr);
    dbg_pr("wake_thread: Domain of %p is %d.\n", sync, domain);
    if(domain == 0 
        // Problem because so far, all stacks in the same domain.
        || domain == DOMAIN_EXTENSION) {
        *(sync->event) = true;
        wake_up_interruptible(sync->wq);
    } else {
        exit_gate();
    }
    do_exit(0);
#else
    // Security not ensured in this case because return address can be abused.
    return;
#endif
}

#ifdef  CONFIG_DIK_THREAD_POOL
struct task_struct * init_thread_pool;
void * init_task_pool_data;
static int call_initfunc(void * data);
DECLARE_WAIT_QUEUE_HEAD(wq);
int stack_domain_changed = 0;
static inline void do_init_thread_pool(void) {
    init_thread_pool = kthread_create(call_initfunc, init_task_pool_data,
        "init_task_pool");
    set_task_stack_domain_id(DOMAIN_EXTENSION, init_thread_pool);
}
#endif

#ifdef CONFIG_DIK_EVA
#include <linux/timekeeping.h>
#include <linux/dik/cyclecount.h>
struct timespec ts_before, ts_after;
unsigned int cc_before, cc_after;
extern unsigned int *cc_done_pt;
#endif
static void thread_and_sync(int (*threadfn)(void *data), void *data,
    const char *namefmt, struct sync_args *sync)
{
#ifdef CONFIG_DIK_USE_THREAD
#ifndef  CONFIG_DIK_THREAD_POOL
    DECLARE_WAIT_QUEUE_HEAD(wq);
#endif
    int event = false;
    unsigned long stack;
    struct task_struct *task;
    sync->wq = &wq;
    sync->event = &event;

    dbg_pr("Using thread for extensions.\n");
#ifdef  CONFIG_DIK_THREAD_POOL
    if(threadfn == call_initfunc) {
        init_task_pool_data = data;
        change_reg_ids();
        stack = set_task_stack_domain_id(DOMAIN_EXTENSION, init_thread_pool);
#ifdef CONFIG_DIK_EVA
        get_cyclecount(cc_before);
#endif
        wake_up_process(init_thread_pool);
    } else {
        task = kthread_create(threadfn, data, namefmt);
        stack = set_task_stack_domain_id(DOMAIN_EXTENSION, task);
        wake_up_process(task);
    }
#else
    task = kthread_create(threadfn, data, namefmt);
    stack = set_task_stack_domain_id(DOMAIN_EXTENSION, task);
    wake_up_process(task);
#endif

    wait_event_interruptible(wq, event != 0);
    
    dbg_pr("call change_stack_back for emulation environment(?)\n");
    change_stack_back(DOMAIN_KERNEL, stack);
#else
    threadfn(data);
#endif
}

/************ initcall wrapper ************/

#include<linux/init.h>

struct initcall_args {
    // these two are specific to each wrapper
    initcall_t fn;
    int ret;

    //synchronisation
    struct sync_args *sync;
};

static void wakeinit_thread(struct initcall_args *args, int local_ret, int *ret) {
    open_close(2);
    entry_gate(wakeinit_label);
    dbg_pr("Post entry gate \\o/ !\n");
    *ret = local_ret;
#ifdef CONFIG_DIK_EVA
    get_cyclecount(*cc_done_pt);
#endif
#ifdef  CONFIG_DIK_THREAD_POOL
    /* This is not very performant because it one thread is created for every
     * init_module call. Just happens after the original function is called.
     * But "simulates" thread pool in term of performance.
     */
    do_init_thread_pool();
#endif
    wake_thread(args->sync, (unsigned int) args);
#ifndef CONFIG_DIK_USE_THREAD
    return;
#endif
wakeinit_label:
    wakeinit_thread(args, local_ret, ret);
}

static int call_initfunc(void * data) {
    struct initcall_args *args;
    initcall_t fn;
    int local_ret;
    int * ret; // return value on previous stack

#ifdef CONFIG_DIK_EVA
    get_cyclecount(cc_after);
    do_posix_clock_monotonic_gettime(&ts_after);
    printk("ts_before_call: %ld.%ld - ts_after_call: %ld.%ld\n",
            ts_before.tv_sec, ts_before.tv_nsec,
            ts_after.tv_sec, ts_after.tv_nsec);
    printk("cc_before_call: %i - cc_after_call: %i\n", cc_before, cc_after);
#endif
    dbg_pr("*********** CALL_INITFUNC ***********\n");
    walk_registers();

#ifdef  CONFIG_DIK_THREAD_POOL
    data = init_task_pool_data;
#endif
    dbg_pr("call_initfunc args. data: %p, ", data);
    args = (struct initcall_args*) data;
    fn = args->fn;
    ret = &(args->ret);
    dbg_pr("fn: %p and ret: %p.\n", fn, ret);

    exit_gate();
    open_close(0);
    local_ret = fn();
    open_close(1);

    wakeinit_thread(args, local_ret, ret);
    return 0;
}

static int thread_initfunc(initcall_t fn) {
    void * data;
    struct initcall_args args;
    struct sync_args sync;

    if(unlikely(!stack_domain_changed)) {
        set_task_stack_domain_id(DOMAIN_EXTENSION, init_thread_pool);
        stack_domain_changed = 1;
    }

    dbg_pr("********** THREAD_INITFUNC **********\n");

    args.fn = fn;
    args.sync = &sync;
    data = (void*) &args;

    thread_and_sync(call_initfunc, data, "call_initfunc", &sync);

    return args.ret;
}

/************ exitcall wrapper ************/
struct exitcall_args {
    void (*fn) (void);
    struct sync_args *sync;
};

static void wakeexit_thread(struct exitcall_args *args) {
    entry_gate(wakeexit_label);
#ifdef  CONFIG_DIK_THREAD_POOL
    wake_thread(args->sync, (unsigned int) args); 
#else
    return;
#endif
wakeexit_label:
    wakeexit_thread(args);
}

static int call_exitfunc(void *data) {
    struct exitcall_args *args;
    void (*fn) (void);

    dbg_pr("*********** CALL_EXITFUNC ***********\n");
    walk_registers();

    args = (struct exitcall_args*) data;
    fn = args->fn;

    exit_gate();
    fn();
    
    wakeexit_thread(args);
    return 0;
}

static void thread_exitfunc(void (*fn) (void)) {
    void * data;
    struct exitcall_args args;
    struct sync_args sync;

    dbg_pr("********** THREAD_EXITFUNC **********\n");

    args.fn = fn;
    args.sync = &sync;
    data = (void*) &args;

    thread_and_sync(call_exitfunc, data, "call_exitfunc", &sync);
}


/******************* module init/exit **********************/

extern void register_wrapper_initcall(void * ptr);
extern void register_wrapper_exitcall(void * ptr);
void register_switchers(void) {
    register_wrapper_initcall(thread_initfunc);
    register_wrapper_exitcall(thread_exitfunc);
}

static int switcher_init(void) {
    pr_debug("domain_switcher module_init.\n");
    register_switchers();
#ifdef  CONFIG_DIK_THREAD_POOL
    init_thread_pool = kthread_create(call_initfunc, init_task_pool_data,
        "init_task_pool");
#endif

    return 0;
}

static void switcher_exit(void) {
    pr_debug("domain_switcher module_exit also has nothing to do.\n");
}

module_init(switcher_init);
module_exit(switcher_exit);
