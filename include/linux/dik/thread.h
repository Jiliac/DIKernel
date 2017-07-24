// This is for debug. Likely to be removed later
extern void change_stack_back(size_t domain_id, unsigned int stack);

extern unsigned long set_task_stack_domain_id(size_t domain_id, struct task_struct *task);
extern void set_task_DACR(size_t new_dacr, struct task_struct *task);
void modify_task_DACR(size_t domain, size_t type, struct task_struct *task);
extern void read_thread_cpu_domain(struct task_struct *task);
extern void read_current_thread_cpu_domain(void);
extern void read_task_ids(struct task_struct *task);
extern void read_current_task_ids(void);
