#include <linux/module.h>
#include<linux/slab.h>
#include <linux/kthread.h>   // for kthreads
#include <linux/wait.h>

MODULE_LICENSE("GPL");

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

// By data is passed initcall_t fn pointer first,
// then returns the int.
// problem if more complicated structure are passed?
#include <linux/dik/stack.h>    // print stack pointer

struct initcall_args {
    // these two are specific to each wrapper
    initcall_t fn;
    int ret;

    //synchronisation
    wait_queue_head_t *wq;
    int *event;
};

int call_initfunc(void * data) {
    struct initcall_args *args;
    initcall_t fn;
    wait_queue_head_t *wq;
    int *event;

    args = (struct initcall_args*) data;
    fn = args->fn;
    wq = args->wq;
    event = args->event;

    print_sp();
    args->ret = fn();

    *event = true;
    wake_up_interruptible(wq);

    do_exit(0);
    return 0;
}

int thread_initfunc(initcall_t fn) {
    void * data;
    struct initcall_args args;
    DECLARE_WAIT_QUEUE_HEAD(wq);
    int event = false;

    args.fn = fn;
    args.wq = &wq;
    args.event = &event;

    /* problem: those arguments are on the stack, and we want to close it domain
     * when going to this other thread...
     * public domain dynamic alloc ?
     * -> YES, this should be in a public domain so it's fine.
     */
    data = (void*) &args;
    kthread_run(call_initfunc, data, "call_initfunc");

    wait_event_interruptible(wq, event != 0);

    // Should check if data is right type.
    return args.ret;
}

/************ exitcall wrapper ************/

// also needs kthread, semaphore and wait queues
// as initcall needs.
void call_exitfunc(void (*fn) (void)) {
    printk("drivers/public_domain/wrapper.c: Just calling the exit "
        "function.\n");
    fn();
}

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
    register_wrapper_exitcall(call_exitfunc);
}

static int wrapper_init(void) {
    printk("drivers/public_domain/wrapper.c: init - registering wrappers.\n");
    register_wrappers();
    return 0;
}

module_init(wrapper_init);
