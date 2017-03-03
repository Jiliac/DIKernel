#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>     // for kmalloc
#include <linux/kmod.h>     // for request_module
#include "table_walk.h" // for modify_domain_id
#include "syms_modif.h"

#define VMALLOC_SIZE   1000000 
#define BUF_SIZE       1000000 

void kmallocing(void) {
    char *buf;
    int i;

    printk("kmalloc-inc to check its allocation process\n");
    buf = kmalloc(BUF_SIZE, GFP_ATOMIC);
    for(i = 0; i < BUF_SIZE / sizeof(char); i++) {
        buf[i] = 42;
    }
}

asmlinkage long sys_dikcall(void) {
    //modif_symbol();
    void * ptr;
    ptr = vmalloc(VMALLOC_SIZE);
    //walk_pgd(ptr);
    modify_domain_id((long unsigned int) ptr, 3);
    vfree(ptr);

    return 0;
}
