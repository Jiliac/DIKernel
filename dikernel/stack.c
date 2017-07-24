#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>    // for init_mm
#include <linux/dik/stack.h>
#include <linux/dik/myprint.h>
#include "table_walk.h"     // for modify_domain_id

#ifdef DEBUG
size_t print_sp(void) {
    size_t sp;
    struct thread_info *thread = current_thread_info();
    read_sp(sp);
    dbg_pr("Current stack pointer: 0x%8x, start: %p, end: %p\n",
        sp, thread, ((void*) thread) + THREAD_START_SP);
    return sp;
}
#else
size_t print_sp(void) {}
#endif
EXPORT_SYMBOL(print_sp);
