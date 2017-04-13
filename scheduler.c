#include "scheduler.h"

queue* tasks;

//TODO let's squitch over for a task struct for clarity. We can store filenames + status of a task. See node.h.
void schedule_task(char* task) {
    if (tasks == NULL)
        tasks = queue_create();
    queue_push(tasks, task);
    tasks->size++;
}

void distribute_task(node* workers) {
  //TODO
}

//Put any tasks the woker was working on back onto the queue
//"Recover" the tasks
void recover_tasks(node* worker) {
  //TODO
}

//Returns the worker node that is least used
node* get_least_used_worker() {
  //TODO
  return NULL;
}

void shutdown_scheduler() {
    queue_destroy(tasks);
    tasks = NULL;
}
