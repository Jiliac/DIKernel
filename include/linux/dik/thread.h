void modify_task_DACR(size_t domain, size_t type, struct task_struct *task);
void read_thread_cpu_domain(struct task_struct *task);
extern void read_task_ids(struct task_struct *task);
extern void read_current_task_ids(void);
