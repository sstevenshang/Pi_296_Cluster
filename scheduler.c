#include <scheduler.h>

queue* tasks;

void schedule_task(char* task) {
    if (tasks == NULL)
        tasks = queue_create();
    queue_push(tasks, task);
    size++;
}

void distribute_task(node* workers) {

}

//Put any tasks the woker was working on back onto the queue
//"Recover" the tasks
void recover_tasks(node* worker) {

}

//Returns the worker node that is least used
node* get_least_used_worker() {

}

void shutdown_scheduler() {
    queue_destroy(tasks);
    tasks = NULL;
}
