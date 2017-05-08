#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>    // for init_mm
#include <linux/dik/stack.h>
#include <linux/dik/myprint.h>
#include "table_walk.h"     // for modify_domain_id

/************** Change Stack Domain ID ************/
void change_stack_domain_id(unsigned int new_id) {
    size_t sp;
    read_sp(sp);
    change_domain_id(sp, new_id);
}

/****************** just for test *****************/
size_t print_sp(void) {
    size_t sp;
    read_sp(sp);
    dbg_pr("Current stack pointer: 0x%8x.\n", sp);
    return sp;
}
EXPORT_SYMBOL(print_sp);
