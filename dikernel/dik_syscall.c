#include <linux/kernel.h>

asmlinkage long sys_dikcall(void) {
    printk("first sys call test\n");
    return 0;
}
