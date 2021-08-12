typedef void (*fptr)();

struct taskQueue {
	int CURRENTSIZE;
	int MAXSIZE;
	struct task *tasks;
};

struct taskQueue ready_queue;
struct taskQueue delay_queue;

struct task {
    int priority;
    fptr pointer_to_func;
    int delay;
};