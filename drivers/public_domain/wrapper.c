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
#include <linux/dik/domain.h>   // DOMAIN_PUBLIC
#include <linux/dik/dacr.h>     // write_dacr and read_dacr macros
#include <asm/domain.h>         // for the macros
#include <asm/thread_info.h>    // for the macros (current_thread_info)

/******* Debug Function *********/
//#define read_pc(pc) asm volatile("mov %0, r15" : "=r" (pc) :)

static void walk_registers(void) {
    size_t reg;

    /*** for programm counter ***/
    //read_pc(pc);
    asm volatile("mov %0, r15" : "=r" (reg) :);
    printk("Current program counter: 0x%8x.\n", reg);
    change_stack_back(20, reg);  // first arg is just random value above
                                // 16 for debug mode

    /*** for stack pointer ***/
    asm volatile("mov %0, r13" : "=r" (reg) :);
    printk("Current stack pointer: 0x%8x.\n", reg);
    change_stack_back(20, reg);
}

/********************************/

struct sync_args {
    wait_queue_head_t *wq;
    int *event;
    size_t domain, old_dacr;
};

#include <asm/tlbflush.h>       // not sure needed to vlush here. For test
static void switch_dacr_to_module(struct sync_args *sync) {
    /*
     * Changing thread cpu_domain
     * i.e. the DACR the kernel use while running this thread
     */
    size_t new_dacr;
    struct thread_info *info = current_thread_info();
    new_dacr = domain_val(sync->domain, DOMAIN_MANAGER) |
            domain_val(DOMAIN_USER, DOMAIN_MANAGER) |
            //domain_val(DOMAIN_KERNEL, DOMAIN_MANAGER) |
            domain_val(DOMAIN_IO, DOMAIN_CLIENT) |
            domain_val(DOMAIN_PUBLIC, DOMAIN_MANAGER);
    printk("New DACR value to be set: 0x%x. Domain id: %i\n", new_dacr, sync->domain);
    read_dacr(sync->old_dacr);
    info->cpu_domain = new_dacr;

    /*******************************
    *** !!!NEED TO BE CAREFUL!!! ***
    ** This instruction shouldn't **
    ** stay as is here. Know how  **
    ** to correct it though.      **
    *(Pointer with fix DACR values)*
    *******************************/
    walk_registers();
    //local_flush_tlb_all();    // Probably useless but just to be sure.
    write_dacr(sync->old_dacr);
    printk("Bug point? 3\n");

    /* This block is to check if problem comes immediately (caused by some
     * pointers in wrong domain right now) or is caused by some other
     * intervention like interrupt, exception or calling code in kernel. 
     */
    write_dacr(new_dacr);
    write_dacr(sync->old_dacr);
    printk("Bug point? 4\n");

    write_dacr(new_dacr);
    printk("Bug point? 5\n");
}

void wake_calling_thread(struct sync_args *sync) {
    printk("In call_initfunc thread. ");
    read_current_task_ids();
    print_sp();

    /* Really needed? Or the kernel will do it by himself when changing thread.
     */
    write_dacr(sync->old_dacr);
    
    *(sync->event) = true;
    wake_up_interruptible(sync->wq);

    do_exit(0);
}

void thread_and_sync(int (*threadfn)(void *data), void *data,
    const char *namefmt, struct sync_args *sync, size_t domain)
{
    DECLARE_WAIT_QUEUE_HEAD(wq);
    int event = false;
    unsigned long stack;
    struct task_struct *task;

    sync->wq = &wq;
    sync->event = &event;
    sync->domain = domain;
    read_current_task_ids();
    read_current_thread_cpu_domain();

    task = kthread_create(threadfn, data, namefmt);
    
    printk("In module wrapper: ");
    read_thread_cpu_domain(task);

    /* 
     * Changing stack domain ID
     * & Enforce this change by TLB flush??? if we do that all the time it's
     * VERY long!
     */
    stack = set_task_stack_domain_id(domain, task);

    printk("In module wrapper, after modif: ");
    read_thread_cpu_domain(task);

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

    switch_dacr_to_module(args->sync);
    printk("Bug point? LAST\n");
    args->ret = fn();

    wake_calling_thread(args->sync);
    return 0;
}

int thread_initfunc(initcall_t fn, size_t domain) {
    void * data;
    struct initcall_args args;
    struct sync_args sync;

    args.fn = fn;
    args.sync = &sync;
    data = (void*) &args;

    walk_registers();

    thread_and_sync(call_initfunc, data, "call_initfunc", &sync, domain);

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

void thread_exitfunc(void (*fn) (void), size_t domain) {
    void * data;
    struct exitcall_args args;
    struct sync_args sync;

    args.fn = fn;
    args.sync = &sync;
    data = (void*) &args;

    thread_and_sync(call_exitfunc, data, "call_exitfunc", &sync, domain);
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
    register_wrapper_exitcall(thread_exitfunc);
}

static int wrapper_init(void) {
    printk("drivers/public_domain/wrapper.c: init - registering wrappers.\n");
    register_wrappers();
    return 0;
}

module_init(wrapper_init);
