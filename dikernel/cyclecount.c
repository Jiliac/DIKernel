#include <linux/kernel.h>
#include <linux/smp.h>

void init_perfcounter(void *data) {
    // in general enable all counters (including cycle counter)
    int32_t value = 1;
    value |= 2;     // reset all counters to zero.
    value |= 4;     // reset cycle counter to zero.
    value |= 16;

    // program the performance-counter control-register:
    asm volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r"(value));

    // enable all counters:  
    asm volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r"(0x8000000f));  

    // clear overflows: 
    asm volatile ("MCR p15, 0, %0, c9, c12, 3\t\n" :: "r"(0x8000000f));
}

void init_perfcounters(void) {
    on_each_cpu(init_perfcounter, NULL, 1);
}
