#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>    // for init_mm
#include <linux/dik/stack.h>
#include "table_walk.h"     // for modify_domain_id

/************** Change Stack Domain ID ************/
void change_stack_domain_id(unsigned int new_id) {
    size_t sp;
    read_sp(sp);
    modify_section_domain_id((unsigned long) sp, new_id);
}

/****************** just for test *****************/
size_t print_sp(void) {
    size_t sp;
    read_sp(sp);
    printk("Current stack pointer: 0x%8x.\n", sp);
    return sp;
}
EXPORT_SYMBOL(print_sp);
