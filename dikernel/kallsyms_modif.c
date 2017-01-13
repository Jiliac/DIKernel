#include <linux/kallsyms.h>
#include "kallsyms_modif.h"

void kallsyms_modif_try() {
    int ret;
    ret = kallsyms_lookup_name("xc2028_attach");
    printk("lookup return: %i\n", ret);
    return;
}
