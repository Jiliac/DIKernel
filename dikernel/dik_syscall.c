#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>     // for kmalloc
#include <linux/kmod.h>     // for request_module
#include <linux/dik/set_wrap.h> // for call_switcher_to_mod
#include "dacr.h"           // for read and write dacr
#include "stack.h"          // for read_sp
#include "table_walk.h"     // for read_ttbr

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
    read_DACR();
    write_DACR(4, 1);
    read_DACR();

    change_stack_test();

    return 0;
}
