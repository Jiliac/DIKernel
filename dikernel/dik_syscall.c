#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/kmod.h>     // for quest_module
#include <linux/slab.h>     // for kmalloc
#include <linux/kmod.h>     // for request_module
#include "table_walk.h" // for modify_domain_id
#include "syms_modif.h"
#include <linux/dik/set_wrap.h> // for call_switcher_to_mod

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

extern void switch_to_module(void);
asmlinkage long sys_dikcall(void) {
    //request_module("domain_switcher");
    call_switcher_to_mod();

    return 0;
}
