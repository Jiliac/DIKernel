#include <linux/slab.h>
#include <linux/vmalloc.h>
#include "table_walk.h"     // for modify_domain_id and section_walking
#include "stack.h"

int fibo_test(int i) {
    switch(i) {
        case 0:
            return 0;
            break;
        case 1:
            return 1;
            break;
        default:
            return fibo_test(i-1) + fibo_test(i-2);
    }
}

void * allocate_stack(unsigned int stack_size, unsigned int id) {
    void * ret;
    ret = kmalloc(stack_size, GFP_KERNEL);
    printk("Kmallocing 0x%p\n", ret);
    //modify_section_domain_id((unsigned long) ret, id);
    //modify_section_domain_id((unsigned long) ret, id);
    return ret;
}

/*
 * - allocate new stack
 * - allocate and save the current stack pointer to restore it later
 * - redirect sp to new stack
 * - do some stuff
 * - restore stack
 * - free the "used" new stack
 */

#define STACK_SIZE 100000
void change_stack_test(void) {
    size_t * new_sp;
    size_t old_sp;

    new_sp = ((size_t*) allocate_stack(STACK_SIZE * sizeof(int), 0))
        + STACK_SIZE;
    read_sp(old_sp);
    printk("dikernel/stack.c: old stack pointer: %8x.\n", old_sp);

    /********** some domain switching should be done here *******/
    write_sp((size_t) new_sp);

    // Maybe it's actually printk creating the bug.
    //printk("We've just changed the stack pointer, hopefully doesn't crash.\n");
    fibo_test(15);

    //read_sp(tmp_sp);
    //printk("dikernel/stack.c: Checking the new stack pointer position: "
    //   "0x%8x.\n", tmp_sp);

    // restoring stack
    write_sp(old_sp);
    kfree(new_sp-STACK_SIZE);
}
