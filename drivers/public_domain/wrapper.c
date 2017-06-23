#include <linux/module.h>
#include <linux/dik/myprint.h>
#include <linux/dik/dacr.h>     // write_dacr and read_dacr macros
#include "gates.h"

MODULE_LICENSE("GPL");

/***************** factorizing wrapper code ***************/
static void post_call(void) {
    exit_gate();
    dbg_pr("After the exit gate, changing DACR to 0x30df to trigger bug.\n");
    write_dacr(0x30df);
}
/****************** data symbol wrappers ******************/

/****************** code symbol wrappers ******************/

#include <linux/slab.h>
#include <linux/slab_def.h>
void * wrapper__kmalloc(size_t size, gfp_t gfp) {
    void * ret;
    entry_gate(wrapper__kmalloc_label);
    ret = __kmalloc(size, gfp);
    change_domain_ext_ptr(ret, (unsigned int) size);
    post_call();
    return ret;
wrapper__kmalloc_label:
    return wrapper__kmalloc(size, gfp);
}
EXPORT_SYMBOL(wrapper__kmalloc);

void wrapper_kfree(const void * ptr) {
    entry_gate(wrapper_kfree_label);
    kfree(ptr);
    exit_gate();
wrapper_kfree_label:
    wrapper_kfree(ptr);
}
EXPORT_SYMBOL(wrapper_kfree);

void *wrapper_kmem_cache_alloc(struct kmem_cache *kc, gfp_t flags) {
    void * ret;
    entry_gate(wrapper_kmem_cache_alloc_label)
    ret = kmem_cache_alloc(kc, flags);
    change_domain_ext_ptr(ret, kc->size);
    exit_gate();
    return ret;
wrapper_kmem_cache_alloc_label:
    return wrapper_kmem_cache_alloc(kc, flags);
}
EXPORT_SYMBOL(wrapper_kmem_cache_alloc);

extern void __aeabi_unwind_cpp_pr0(void);
void wrapper__aeabi_unwind_cpp_pr0(void) {
    entry_gate(wrapper__aeabi_unwind_cpp_pr0_label);
    __aeabi_unwind_cpp_pr0();
    exit_gate();
    return;
wrapper__aeabi_unwind_cpp_pr0_label:
    wrapper__aeabi_unwind_cpp_pr0();
}
EXPORT_SYMBOL(wrapper__aeabi_unwind_cpp_pr0);

extern void __aeabi_unwind_cpp_pr1(void);
void wrapper__aeabi_unwind_cpp_pr1(void) {
    entry_gate(wrapper__aeabi_unwind_cpp_pr1_label);
    dbg_pr("Calling __aeabi_unwind_cpp_pr1 through a wrapper.\n");
    __aeabi_unwind_cpp_pr1();
    exit_gate();
    return;
wrapper__aeabi_unwind_cpp_pr1_label:
    wrapper__aeabi_unwind_cpp_pr1();
}
EXPORT_SYMBOL(wrapper__aeabi_unwind_cpp_pr1);

int wrapper_vprintk(const char *fmt, va_list args) {
    int ret;
    entry_gate(wrapper_vprintk_label);
    dbg_pr("In wrapper_vprintk\n");
    ret = vprintk(fmt, args);
    exit_gate();
    return ret;
wrapper_vprintk_label:
    return wrapper_vprintk(fmt, args);
}
EXPORT_SYMBOL(wrapper_vprintk);

int wrapper_printk(const char *fmt, ...) {
    va_list args;
    int r;

    va_start(args, fmt);
    /* This remove some of the flexibility of the printk function provided by
     * the kernel. Not very important here. Could still be done by more sharing.
     */
    r = wrapper_vprintk(fmt, args);
    va_end(args);
    return r;
}
EXPORT_SYMBOL(wrapper_printk);

#include <linux/device.h>
void wrapper___dev_printk(const char * level, const struct device *dev,
    struct va_format *vaf)
{
    entry_gate(wrapper___dev_printk_label)
    __dev_printk(level, dev, vaf);
    exit_gate();
wrapper___dev_printk_label:
    wrapper___dev_printk(level, dev, vaf);
}
EXPORT_SYMBOL(wrapper___dev_printk);

#define define_wrapper_dev_printk_level(func, kern_level)   \
void func(const struct device *dev, const char *fmt, ...)	\
{								\
	struct va_format vaf;					\
	va_list args;						\
								\
	va_start(args, fmt);					\
								\
	vaf.fmt = fmt;						\
	vaf.va = &args;						\
								\
	wrapper___dev_printk(kern_level, dev, &vaf);    \
								\
	va_end(args);						\
}								\
EXPORT_SYMBOL(func);

define_wrapper_dev_printk_level(wrapper_dev_err, KERN_ERR);

#include <linux/platform_device.h>
int wrapper__platform_driver_register(struct platform_driver *pd,
    struct module *mod) {
    int ret;
    entry_gate(wrapper__platform_driver_register_label)
    ret = __platform_driver_register(pd, mod);
    exit_gate();
    return ret;
wrapper__platform_driver_register_label:
    return wrapper__platform_driver_register(pd, mod);
}
EXPORT_SYMBOL(wrapper__platform_driver_register);

void wrapper_platform_driver_unregister(struct platform_driver *pd) {
    entry_gate(wrapper_platform_driver_unregister_label);
    platform_driver_unregister(pd);
    exit_gate();
wrapper_platform_driver_unregister_label:
    wrapper_platform_driver_unregister(pd);
}
EXPORT_SYMBOL(wrapper_platform_driver_unregister);

#include <linux/clk-provider.h>
#define CLK_SIZE    200*sizeof(int)
struct clk *wrapper_of_clk_src_simple_get(struct of_phandle_args *clkspec,
    void *data) {
    struct clk *ret;
    struct clk *copy_ret;
    entry_gate(wrapper_of_clk_src_simple_get_label);
    ret = of_clk_src_simple_get(clkspec, data);
    // arbitrary size :(
    copy_ret = (struct clk*) kmalloc(CLK_SIZE, GFP_KERNEL);
    memcpy(copy_ret, ret, CLK_SIZE);
    change_domain_ext_ptr((void*)copy_ret, CLK_SIZE);
    exit_gate();
    return copy_ret;
wrapper_of_clk_src_simple_get_label:
    return wrapper_of_clk_src_simple_get(clkspec, data);
}
EXPORT_SYMBOL(wrapper_of_clk_src_simple_get);

struct clk *wrapper_devm_clk_register(struct device *dev, struct clk_hw *hw) {
    struct clk *ret;
    struct clk *copy_ret;
    entry_gate(wrapper_devm_clk_register_label);
    ret = devm_clk_register(dev, hw);
    copy_ret = (struct clk*) kmalloc(CLK_SIZE, GFP_KERNEL);
    memcpy(copy_ret, ret, CLK_SIZE);
    change_domain_ext_ptr((void*)copy_ret, CLK_SIZE);
    exit_gate();
    return copy_ret;
wrapper_devm_clk_register_label:
    return wrapper_devm_clk_register(dev, hw);
}
EXPORT_SYMBOL(wrapper_devm_clk_register);


int wrapper_of_clk_add_provider(struct device_node *np,
             struct clk *(*clk_src_get)(struct of_phandle_args *args,
                            void *data),
             void *data) {
    int ret;
    entry_gate(wrapper_of_clk_add_provider_label);
    ret = of_clk_add_provider(np, clk_src_get, data);
    exit_gate();
    return ret;
wrapper_of_clk_add_provider_label:
    return wrapper_of_clk_add_provider(np, clk_src_get, data);
}
EXPORT_SYMBOL(wrapper_of_clk_add_provider);

void wrapper_of_clk_del_provider(struct device_node *np) {
    entry_gate(wrapper_of_clk_del_provider_label);
    of_clk_del_provider(np);
    exit_gate();
wrapper_of_clk_del_provider_label:
    wrapper_of_clk_del_provider(np);
}
EXPORT_SYMBOL(wrapper_of_clk_del_provider);

/*********************** init function **********************/

static int wrapper_init(void) {
    dbg_pr("drivers/public_domain/wrapper.c: init - registering wrappers.\n");
    return 0;
}

module_init(wrapper_init);
