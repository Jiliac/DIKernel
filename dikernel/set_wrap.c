#include<linux/dik/set_wrap.h>
#include <linux/dik/myprint.h>
#include "syms_modif.h"

/****************** initcall wrapper setup ************/
int (*wrapper_initcall) (initcall_t fn) = NULL;
void register_wrapper_initcall(void * ptr) {
    dbg_pr("dikernel/set_wrap.c: register_wrapper_initcall called.\n");
    wrapper_initcall = ptr;
}
EXPORT_SYMBOL(register_wrapper_initcall);

int call_wrapper_initcall(initcall_t fn) {
    int ret;
    if(!wrapper_initcall)
        return fn();
    ret = wrapper_initcall(fn);
    return ret;
}

/****************** exitcall wrapper setup ************/
void (*wrapper_exitcall) (void (*fn) (void)) = NULL;
void register_wrapper_exitcall(void * ptr) {
    dbg_pr("dikernel/set_wrap.c: register_wrapper_exitcall called.\n");
    wrapper_exitcall = ptr;
}
EXPORT_SYMBOL(register_wrapper_exitcall);

void call_wrapper_exitcall(void (*fn) (void)) {
    if(!wrapper_exitcall)
        fn();
    else
        wrapper_exitcall(fn);
}

/*************** Symbols to wrap definition ***************/

struct code_wrapper {
    char * target_name;
    char * sym_to_change;
};

static struct code_wrapper code_wrappers[] = {
    {"__kmalloc",   "wrapper__kmalloc"  },
    {"kfree",       "wrapper_kfree" },
    {"kmem_cache_alloc",    "wrapper_kmem_cache_alloc"  },
    {"__aeabi_unwind_cpp_pr0",      "wrapper__aeabi_unwind_cpp_pr0" },
    {"__aeabi_unwind_cpp_pr1",      "wrapper__aeabi_unwind_cpp_pr1" },
    {"vprintk",     "wrapper_vprintk"   },
    {"printk",      "wrapper_printk"    },
    {"__dev_printk",    "wrapper___dev_printk"  },
    {"dev_err"  ,   "wrapper_dev_err"   },
    {"platform_driver_unregister",  "wrapper_platform_driver_unregister"    },
    {"__platform_driver_register",  "wrapper__platform_driver_register"     },
    {"of_clk_src_simple_get",       "wrapper_of_clk_src_simple_get"     },
    {"devm_clk_register",       "wrapper_devm_clk_register"     },
    {"of_clk_add_provider",     "wrapper_of_clk_add_provider"   },
    {"of_clk_del_provider", "wrapper_of_clk_del_provider"   },
};

struct data_wrapper {
    char * target_name;
    char * sym_to_change;
    struct kernel_symbol * kernel_sym;
    struct kernel_symbol * wrapper_sym;
};

static struct data_wrapper data_wrappers[] = {};

/**************************** Wrapping Data Symbols ***************************/
void data_wrappers_init(void) {
    unsigned i;
    struct kernel_symbol * kernel_sym;
    struct kernel_symbol * wrapper_sym;
    struct module * mod = NULL;

    mod = find_module(WRAPPER_MODULE);
    for(i=0; i < ARRAY_SIZE(data_wrappers); ++i) {    
        kernel_sym = (struct kernel_symbol*) find_symbol(
            data_wrappers[i].target_name, NULL, 
            NULL, true, true);
        wrapper_sym = (struct kernel_symbol*) find_symbol(
            data_wrappers[i].sym_to_change, &mod,
            NULL, true, true);
        if(kernel_sym && wrapper_sym) {
            dbg_pr("Symbol value in kernel: %lx, and in wrapper: %lx.\n", 
                kernel_sym->value, wrapper_sym->value);
            data_wrappers[i].kernel_sym = kernel_sym;
            data_wrappers[i].wrapper_sym = wrapper_sym;
            kernel_sym->value = wrapper_sym->value;
        } else {
            dbg_pr("Couldn't find %s symbol.\n", data_wrappers[i].target_name);
        }
    }
}

void update_data_wrappers(void) {
}

/************************ Setting Wrappers and utility ************************/

/* @target_name: the base kernel symbol whose access we want to interpose.
 * @sym_to_change: the symbol that we interpose in between base kernel and
 *      extension.
 */
void modify_sym_in_mod(struct module * mod, char * target_name, char * sym_to_change) {
    const struct kernel_symbol * sym;

    if(mod) {
        sym = find_symbol(sym_to_change, &mod, NULL, true, true);
        if(sym) {
            dbg_pr("We found %s symbol ", sym_to_change);
            modify_symbol(target_name, sym->value);
        }
        else
            dbg_pr("dik/wrapper.c:setting_wrappers couldn't find %s symbol to "
                    "modify.\n", sym_to_change);
    }
    else
        dbg_pr("Argument mod was passed with NULL address.\n");
}

int wrapper_set = 0;    // boolean value
void setting_wrappers() {
    unsigned i;
    struct module * mod = NULL;
    dbg_pr("!!!!!!!!!! setting_wrappers start !!!!!!!!!!\n");
    if(!request_module(SWITCHER_MODULE)) {
        mod = find_module(WRAPPER_MODULE);

        wrapper_set = 1;    // WRAPPER_MODULE should be in DOMAIN_PUBLIC
        dbg_pr("setting_wrapper: %s module loaded.\n", WRAPPER_MODULE);

        /* Here we should change the domain ID of the Public Domain.
         */

        for(i = 0; i < ARRAY_SIZE(code_wrappers); ++i) {
            modify_sym_in_mod(mod,
                code_wrappers[i].target_name,
                code_wrappers[i].sym_to_change);
        }
    } else
        dbg_pr("Couldn't load %s module.\n", WRAPPER_MODULE);

    data_wrappers_init();

    dbg_pr("!!!!!!!!!! setting_wrappers end !!!!!!!!!!\n");
}

int get_wrapper_set(void) {
    return wrapper_set;
}
