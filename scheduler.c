#include <scheduler.h>

void schedule_task(char* task) {
    if (tasks == NULL)
        tasks = queue_create();
    queue_push(tasks, task);
    size++;
}

void distribute_task(node* workers) {

}

void shutdown_scheduler() {
    queue_destroy(tasks);
    tasks = NULL;
}
