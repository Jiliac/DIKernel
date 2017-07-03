#include <linux/module.h>
#include <linux/dik/myprint.h>
#include <linux/dik/dacr.h>     // write_dacr and read_dacr macros
#include "gates.h"

MODULE_LICENSE("GPL");

/**********************************************************/
/***************** factorizing wrapper code ***************/

// Only still here for test purpose. Got replaced by exit_gate.
//static void post_call(void) {
//    exit_gate();
//    dbg_pr("After the exit gate, changing DACR to 0x30df to trigger bug.\n");
//    write_dacr(0x30df);
//}

/**********************************************************/
/****************** data symbol wrappers ******************/

#include <linux/slab.h>
#include <linux/slab_def.h>
#include <linux/dik/set_wrap.h>
struct kmem_cache *wrapper_kmalloc_caches[KMALLOC_SHIFT_HIGH + 1];
#define     CACHE_SIZE      sizeof(struct kmem_cache)

void wrapper_kmalloc_caches_init(void) {
    unsigned i;
    void * ptr;
    const unsigned total_size = CACHE_SIZE * (KMALLOC_SHIFT_HIGH+1);

    ptr = kmalloc(total_size, GFP_KERNEL);
    change_domain_ext_ptr(ptr, (unsigned) (total_size/sizeof(unsigned)));

    for(i=0; i<KMALLOC_SHIFT_HIGH + 1; ++i) {
        if(kmalloc_caches[i]) {
            wrapper_kmalloc_caches[i] = (struct kmem_cache*) ptr;
            ptr += CACHE_SIZE;
            memcpy(wrapper_kmalloc_caches[i], kmalloc_caches[i], CACHE_SIZE);
        }
    }
}

void wrapper_kmalloc_caches_update(void) {
    unsigned i;
    for(i=0; i<KMALLOC_SHIFT_HIGH + 1; ++i) {
        if(kmalloc_caches[i]) {
            memcpy(wrapper_kmalloc_caches[i], kmalloc_caches[i], CACHE_SIZE);
        }
    }
}

void wrapper_kmalloc_caches_exit(void) {
    unsigned i;
    for(i=0; i<KMALLOC_SHIFT_HIGH + 1; ++i) {
        if(kmalloc_caches[i]) {
            kfree(kmalloc_caches[i]);
            return;
        }
    }
}

/*********************** init function **********************/
#include "wrapper.h"

static int wrapper_init(void) {
    unsigned i;
    for(i=0; i < ARRAY_SIZE(data_wrapper_info); ++i) {
        modify_data_sym(data_wrapper_info[i].data_sym_name,
                data_wrapper_info[i].new_value);
        if(data_wrapper_info[i].init_wrapper) 
            data_wrapper_info[i].init_wrapper();
    }
    return 0;
}

static void wrapper_exit(void) {
    unsigned i;
    for(i=0; i < ARRAY_SIZE(data_wrapper_info); ++i) {
        if(data_wrapper_info[i].update_wrapper)
            data_wrapper_info[i].update_wrapper();
    }
}

module_init(wrapper_init);
module_exit(wrapper_exit);

/**********************************************************/
/****************** code symbol wrappers ******************/

void * wrapper__kmalloc(size_t size, gfp_t gfp) {
    void * ret;
    entry_gate(wrapper__kmalloc_label);
    ret = __kmalloc(size, gfp);
    change_domain_ext_ptr(ret, (unsigned int) size);
    wrapper_kmalloc_caches_update();
    exit_gate();
    return ret;
wrapper__kmalloc_label:
    return wrapper__kmalloc(size, gfp);
}
EXPORT_SYMBOL(wrapper__kmalloc);

void wrapper_kfree(const void * ptr) {
    entry_gate(wrapper_kfree_label);
    kfree(ptr);
    wrapper_kmalloc_caches_update();
    exit_gate();
    return;
wrapper_kfree_label:
    wrapper_kfree(ptr);
}
EXPORT_SYMBOL(wrapper_kfree);

void *wrapper_kmem_cache_alloc(struct kmem_cache *kc, gfp_t flags) {
    void * ret;
    entry_gate(wrapper_kmem_cache_alloc_label);
    ret = kmem_cache_alloc(kc, flags);
    change_domain_ext_ptr(ret, kc->size);
    wrapper_kmalloc_caches_update();
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
    entry_gate(wrapper___dev_printk_label);
    dbg_pr("wrapper___dev_printk\n");
    __dev_printk(level, dev, vaf);
    exit_gate();
    return;
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
define_wrapper_dev_printk_level(wrapper__dev_info, KERN_INFO);

struct class * __must_check wrapper__class_create(struct module *owner,
        const char *name, struct lock_class_key *key) {
    struct class * ret;
    struct class * copy_ret;
    entry_gate(wrapper__class_create_label);
    ret = __class_create(owner, name, key);
    copy_ret = kmalloc(sizeof(struct class), GFP_KERNEL);
    memcpy(copy_ret, ret, sizeof(struct class));
    exit_gate();
    return copy_ret;
wrapper__class_create_label:
    return wrapper__class_create(owner, name, key);
}
EXPORT_SYMBOL(wrapper__class_create);

void wrapper_class_destroy(struct class *cls) {
    entry_gate(wrapper_class_destroy_label);
    class_destroy(cls);
    exit_gate();
    return;
wrapper_class_destroy_label:
    return wrapper_class_destroy(cls);
}
EXPORT_SYMBOL(wrapper_class_destroy);

struct device *wrapper_device_create_vargs(struct class *class,
        struct device *parent, dev_t devt, void *drvdata, const char *fmt,
        va_list args) {
    struct device *ret;
    struct device *copy_ret;
    entry_gate(wrapper_device_create_vargs_label);
    ret = device_create_vargs(class, parent, devt, drvdata, fmt, args);
    /*  Problem of 'just' copying the device structure is that is points to many
     *  other structure. However, it is mostly used by modules just to call
     *  other kernel functions in the end.
     */
    copy_ret = kmalloc(sizeof(struct device), GFP_KERNEL);
    memcpy(copy_ret, ret, sizeof(struct device));
    exit_gate();
    return copy_ret;
 wrapper_device_create_vargs_label:
    return wrapper_device_create_vargs(class, parent, devt, drvdata, fmt, args);
}
EXPORT_SYMBOL(wrapper_device_create_vargs);

struct device *wrapper_device_create(struct class *class, struct device *parent,
        dev_t devt, void *drvdata, const char *fmt, ...) {
    va_list vargs;
    struct device *dev;

    va_start(vargs, fmt);
    dev = wrapper_device_create_vargs(class, parent, devt, drvdata, fmt, vargs);
    va_end(vargs);
    return dev;

}
EXPORT_SYMBOL(wrapper_device_create);

#include <linux/platform_device.h>
int wrapper__platform_driver_register(struct platform_driver *pd,
    struct module *mod) {
    int ret;
    entry_gate(wrapper__platform_driver_register_label);
    dbg_pr("__platform_driver_register\n");
    ret = __platform_driver_register(pd, mod);
    exit_gate();
    return ret;
wrapper__platform_driver_register_label:
    return wrapper__platform_driver_register(pd, mod);
}
EXPORT_SYMBOL(wrapper__platform_driver_register);

void wrapper_platform_driver_unregister(struct platform_driver *pd) {
    entry_gate(wrapper_platform_driver_unregister_label);
    dbg_pr("wrapper_platform_driver_unregister called.\n");
    platform_driver_unregister(pd);
    exit_gate();
    return;
wrapper_platform_driver_unregister_label:
    wrapper_platform_driver_unregister(pd);
}
EXPORT_SYMBOL(wrapper_platform_driver_unregister);

struct resource *wrapper_platform_get_resource(struct platform_device *pd,
        unsigned int i1, unsigned int i2) {
    struct resource *ret;
    struct resource *copy_ret;
    entry_gate(wrapper_platform_get_resource_label);
    ret = platform_get_resource(pd, i1, i2);
    copy_ret = kmalloc(sizeof(struct resource), GFP_KERNEL);
    memcpy(copy_ret, ret, sizeof(struct resource));
    exit_gate();
    return copy_ret;
wrapper_platform_get_resource_label:
    return wrapper_platform_get_resource(pd, i1, i2);
}
EXPORT_SYMBOL(wrapper_platform_get_resource);

void wrapper_device_destroy(struct class *cls, dev_t devt) {
    entry_gate(wrapper_device_destroy_label);
    device_destroy(cls, devt);
    exit_gate();
    return;
wrapper_device_destroy_label:
    wrapper_device_destroy(cls, devt);
}
EXPORT_SYMBOL(wrapper_device_destroy);

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
    return;
wrapper_of_clk_del_provider_label:
    wrapper_of_clk_del_provider(np);
}
EXPORT_SYMBOL(wrapper_of_clk_del_provider);

#include <linux/hw_random.h>
int wrapper_hwrng_register(struct hwrng *rng) {
    int ret;
    entry_gate(wrapper_hwrng_register_label);
    dbg_pr("hwrng_register\n");
    ret = hwrng_register(rng);
    dbg_pr("hwrng_register post ret\n");
    exit_gate();
    return ret;
wrapper_hwrng_register_label:
    return wrapper_hwrng_register(rng);
}
EXPORT_SYMBOL(wrapper_hwrng_register);

void wrapper_hwrng_unregister(struct hwrng *rng) {
    entry_gate(wrapper_hwrng_unregister_label);
    dbg_pr("hwrng_unregister\n");
    hwrng_unregister(rng);
    exit_gate();
    return;
wrapper_hwrng_unregister_label:
    wrapper_hwrng_unregister(rng);
}
EXPORT_SYMBOL(wrapper_hwrng_unregister);

#include <asm/io.h>
void wrapper__arm_iounmap(volatile void __iomem *addr) {
    entry_gate(wrapper__arm_iounmap_label);
    dbg_pr("__arm_iounmap\n");
    __arm_iounmap(addr);
    exit_gate();
    return;
wrapper__arm_iounmap_label:
    wrapper__arm_iounmap(addr);
}
EXPORT_SYMBOL(wrapper__arm_iounmap);

#include <linux/of_address.h>
/* !!! BEWARE !!!!
 * The passed pointer probably is a pointer to some data that will be read.
 * This wrapper isn't finished...
 */
void __iomem *wrapper_of_iomap(struct device_node *node, int index) {
    void * ret;
    entry_gate(wrapper_of_iomap_label);
    dbg_pr("of_iomap\n");
    ret = of_iomap(node, index);
    exit_gate();
    return ret;
wrapper_of_iomap_label:
    return wrapper_of_iomap(node, index);
}
EXPORT_SYMBOL(wrapper_of_iomap);

#include <linux/fs.h>
int wrapper_alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,
        const char *name) {
    int ret;
    entry_gate(wrapper_alloc_chrdev_region_label);
    ret = alloc_chrdev_region(dev, baseminor, count, name);
    exit_gate();
    return ret;
wrapper_alloc_chrdev_region_label:
    return wrapper_alloc_chrdev_region(dev, baseminor, count, name);
}
EXPORT_SYMBOL(wrapper_alloc_chrdev_region);

void wrapper_unregister_chrdev_region(dev_t dev, unsigned i) {
    entry_gate(wrapper_unregister_chrdev_region_label);
    unregister_chrdev_region(dev, i);
    exit_gate();
    return;
wrapper_unregister_chrdev_region_label:
    wrapper_unregister_chrdev_region(dev, i);
}
EXPORT_SYMBOL(wrapper_unregister_chrdev_region);

#include <linux/cdev.h>
int wrapper_cdev_add(struct cdev *cd, dev_t d, unsigned i) {
    int ret;
    entry_gate(wrapper_cdev_add_label);
    ret = cdev_add(cd, d, i);
    exit_gate();
    return ret;
wrapper_cdev_add_label:
    return wrapper_cdev_add(cd, d, i);
}
EXPORT_SYMBOL(wrapper_cdev_add);

void wrapper_cdev_init(struct cdev *cdev, const struct file_operations *fo) {
    entry_gate(wrapper_cdev_init_label);
    cdev_init(cdev, fo);
    exit_gate();
    return;
wrapper_cdev_init_label:
    wrapper_cdev_init(cdev, fo);
}
EXPORT_SYMBOL(wrapper_cdev_init);

#include <linux/mm.h>
int wrapper_remap_pfn_range(struct vm_area_struct *va, unsigned long addr,
        unsigned long pfn, unsigned long size, pgprot_t prot) {
    int ret;
    entry_gate(wrapper_remap_pfn_range_label);
    /*  @prot argument could/should be checked since it ends up in page tables
     *  that we want to protect.
     */
    ret = remap_pfn_range(va, addr, pfn, size, prot);
    exit_gate();
    return ret;
 wrapper_remap_pfn_range_label:
    return wrapper_remap_pfn_range(va, addr, pfn, size, prot);
}
EXPORT_SYMBOL(wrapper_remap_pfn_range);

#include <asm/pgtable.h>
extern pgprot_t wrapper_phys_mem_access_prot(struct file *file,
        unsigned long pfn, unsigned long size, pgprot_t vma_prot) {
    pgprot_t ret;
    entry_gate(wrapper_phys_mem_access_prot_label);
    ret = phys_mem_access_prot(file, pfn, size, vma_prot);
    exit_gate();
    return ret;
wrapper_phys_mem_access_prot_label:
    return wrapper_phys_mem_access_prot(file, pfn, size, vma_prot);
}
EXPORT_SYMBOL(wrapper_phys_mem_access_prot);
