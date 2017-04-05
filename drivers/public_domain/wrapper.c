#include <linux/module.h>
#include<linux/slab.h>
#include <linux/kthread.h>   // for kthreads
#include <linux/wait.h>

MODULE_LICENSE("GPL");

/************************************************************************/
/* Factorizing as much code as possible  for exit and init call wrapper */
/************************************************************************/
#include <linux/dik/stack.h>    // print stack pointer
#include <linux/dik/thread.h>   // read_current_task_ids

struct sync_args {
    wait_queue_head_t *wq;
    int *event;
};

void wake_calling_thread(struct sync_args *sync) {
    printk("In call_initfunc thread. ");
    read_current_task_ids();
    print_sp();

    *(sync->event) = true;
    wake_up_interruptible(sync->wq);

    do_exit(0);
}

void thread_and_sync(int (*threadfn)(void *data), void *data,
    const char *namefmt, struct sync_args *sync)
{
    DECLARE_WAIT_QUEUE_HEAD(wq);
    int event = false;

    sync->wq = &wq;
    sync->event = &event;
    read_current_task_ids();
    kthread_run(threadfn, data, namefmt);

    wait_event_interruptible(wq, event != 0);
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

// Problem if more complicated structure are in return?

struct initcall_args {
    // these two are specific to each wrapper
    initcall_t fn;
    int ret;

    //synchronisation
    struct sync_args *sync;
};

int call_initfunc(void * data) {
    struct initcall_args *args;
    initcall_t fn;

    args = (struct initcall_args*) data;
    fn = args->fn;
    args->ret = fn();

    wake_calling_thread(args->sync);
    return 0;
}

int thread_initfunc(initcall_t fn) {
    void * data;
    struct initcall_args args;
    struct sync_args sync;

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

int call_exitfunc(void *data) {
    struct exitcall_args *args;
    void (*fn) (void);

    args = (struct exitcall_args*) data;
    fn = args->fn,
    fn();
    
    wake_calling_thread(args->sync);
    return 0;
}

void thread_exitfunc(void (*fn) (void)) {
    void * data;
    struct exitcall_args args;
    struct sync_args sync;

    args.fn = fn;
    args.sync = &sync;
    data = (void*) &args;

    thread_and_sync(call_exitfunc, data, "call_exitfunc", &sync);
}

// also needs kthread, semaphore and wait queues
// as initcall needs.
//void call_exitfunc(void (*fn) (void)) {
//    printk("drivers/public_domain/wrapper.c: Just calling the exit "
//        "function.\n");
//    fn();
//}

/****************** some symbol wrappers ******************/

void * wrapper___kmalloc(size_t size, gfp_t gfp) {
    printk("Calling __kmalloc, but through a wrapper.\n");
    return __kmalloc(size, gfp);
}
EXPORT_SYMBOL(wrapper___kmalloc);

extern void __aeabi_unwind_cpp_pr1(void);
void wrapper___aeabi_unwind_cpp_pr1(void) {
    printk("Calling __aeabi_unwind_cpp_pr1 through a wrapper.\n");
    __aeabi_unwind_cpp_pr1();
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
    printk("drivers/public_domain/wrapper.c: init - registering wrappers.\n");
    register_wrappers();
    return 0;
}

module_init(wrapper_init);
