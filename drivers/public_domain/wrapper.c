#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kthread.h>  // for kthreads
#include <linux/wait.h>
#include <linux/dik/myprint.h>

MODULE_LICENSE("GPL");

/************************************************************************/
/* Factorizing as much code as possible  for exit and init call wrapper */
/************************************************************************/
#include <linux/dik/stack.h>    // print stack pointer
#include <linux/dik/thread.h>   // read_current_task_ids
#include <linux/dik/domain.h>   // DOMAIN_PUBLIC
#include <linux/dik/dacr.h>     // write_dacr and read_dacr macros
#include <asm/domain.h>         // for the macros
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
#endif
/********************************/

struct sync_args {
#ifdef CONFIG_DIK_USE_THREAD
    wait_queue_head_t *wq;
    int *event;
#endif
};

#include <asm/tlbflush.h>       // not sure needed to vlush here. For test
#include <asm/cacheflush.h>
#include "gates.h"

#ifdef CONFIG_VIRTUAL_BK_DID
// To Remove code. Let it in case this hypothesis if true in the end.
extern void change_all_ids(unsigned int id);
extern void change_kernel_domain(void);
#endif

void wake_calling_thread(struct sync_args *sync) {
    entry_gate();
#ifdef CONFIG_DIK_USE_THREAD
    *(sync->event) = true;
    wake_up_interruptible(sync->wq);
    do_exit(0);
#endif
}

void thread_and_sync(int (*threadfn)(void *data), void *data,
    const char *namefmt, struct sync_args *sync)
{
#ifdef CONFIG_DIK_USE_THREAD
    DECLARE_WAIT_QUEUE_HEAD(wq);
    int event = false;
    unsigned long stack;
    struct task_struct *task;
    sync->wq = &wq;
    sync->event = &event;
#endif

#ifdef CONFIG_DIK_USE_THREAD
    dbg_pr("Using thread for extensions.\n");
    task = kthread_create(threadfn, data, namefmt);
    
    /* 
     * Changing stack domain ID
     * & Enforce this change by TLB flush??? if we do that all the time it's
     * VERY long!
     */
    stack = set_task_stack_domain_id(DOMAIN_EXTENSION, task);

    wake_up_process(task);

    wait_event_interruptible(wq, event != 0);
    /*
     * Design wise change the Domain ID of the stack back to its original value
     * isn't a problem. HOWEVER, it is a big performance problem. Because of the
     * TLB flushes... that the whole project was suppose to avoid in the first
     * place! Thus, we haven't really solve the stack problem.
     * 
     * SOLUTION @TODO: Having "pool" of memory that are in a specific somain a
     * being able to allocate stacks in this specific domain. OR having a 1st
     * level page descriptor being created just for this stack. Even with this
     * last solution, we would have to ensure the kernel doesn't modify the
     * domain back or doesn't mistakenly allocate stuff there that isn't
     * supposed to be in this domain. 
     */
    change_stack_back(DOMAIN_KERNEL, stack);
#else
    threadfn(data);
#endif
}

/************ initcall wrapper ************/

#include<linux/init.h>

/*
 * - Create k thread (this includes domain management).
 * - Sleep until waiting queue completion
 */

/* one more function needed.
 * - One to create thread.
 * - One to be called first by the thread and get the return value
 *      in the (*void data) pointer.
 */

struct initcall_args {
    // these two are specific to each wrapper
    initcall_t fn;
    int ret;

    //synchronisation
    struct sync_args *sync;
};

static int call_initfunc(void * data) {
    struct initcall_args *args;
    initcall_t fn;

    dbg_pr("*********** CALL_INITFUNC ***********\n");

    args = (struct initcall_args*) data;
    fn = args->fn;

    exit_gate();
    args->ret = fn();

    wake_calling_thread(args->sync);
    return 0;
}

static int thread_initfunc(initcall_t fn) {
    void * data;
    struct initcall_args args;
    struct sync_args sync;

    dbg_pr("********** THREAD_INITFUNC **********\n");

    args.fn = fn;
    args.sync = &sync;
    data = (void*) &args;

#ifdef DEBUG
    walk_registers();
#endif

    thread_and_sync(call_initfunc, data, "call_initfunc", &sync);

    return args.ret;
}

/************ exitcall wrapper ************/
struct exitcall_args {
    void (*fn) (void);
    struct sync_args *sync;
};

static int call_exitfunc(void *data) {
    struct exitcall_args *args;
    void (*fn) (void);

    dbg_pr("*********** CALL_EXITFUNC ***********\n");
    args = (struct exitcall_args*) data;
    fn = args->fn;

    exit_gate();
    fn();
    
    wake_calling_thread(args->sync);
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

/***************** factorizing wrapper code ***************/
static void pre_call(void){
    entry_gate();
}

static void post_call(void) {
    exit_gate();
    dbg_pr("After the exit gate, changing DACR to 0x30df to trigger bug.\n");
    write_dacr(0x30df);
}

/****************** some symbol wrappers ******************/

void * wrapper___kmalloc(size_t size, gfp_t gfp) {
    void * ret;
    pre_call();
    dbg_pr("Calling __kmalloc, but through a wrapper.\n");
    ret = __kmalloc(size, gfp);
    dbg_pr("Called __kmalloc through a wrapper.\n");
    post_call();
    return ret;
}
EXPORT_SYMBOL(wrapper___kmalloc);

extern void __aeabi_unwind_cpp_pr1(void);
void wrapper___aeabi_unwind_cpp_pr1(void) {
    pre_call();
    dbg_pr("Calling __aeabi_unwind_cpp_pr1 through a wrapper.\n");
    __aeabi_unwind_cpp_pr1();
    post_call();
}
EXPORT_SYMBOL(wrapper___aeabi_unwind_cpp_pr1);

/************ standard module stuff ******************/

extern void register_wrapper_initcall(void * ptr);
extern void register_wrapper_exitcall(void * ptr);
void register_wrappers(void) {
    register_wrapper_initcall(thread_initfunc);
    register_wrapper_exitcall(thread_exitfunc);
}

static int wrapper_init(void) {
    dbg_pr("drivers/public_domain/wrapper.c: init - registering wrappers.\n");
    register_wrappers();
    return 0;
}

module_init(wrapper_init);
