void init_perfcounters(void);

#define get_cyclecount(t)   \
        asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(t))

