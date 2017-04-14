#include "scheduler.h"

queue* tasks;

//TODO let's squitch over for a task struct for clarity. We can store filenames + status of a task. See node.h.
void schedule_task(task* task) {
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
    task** cur = worker->tasks;
    while (*cur != NULL) {
        queue_push(tasks, *cur);
        *cur = NULL;
        cur++;
    }
}

//Returns the worker node that is least used
node* get_least_used_worker(node* workers) {

    node* cur = workers;
    node* least_used = NULL;
    double load_factor;
    double least_load_factor = 1000;

    while (cur != NULL) {
        if (cur->num_of_task > 5) {
            continue;
        }
        load_factor = (cur->cur_load) + (cur->num_of_task * 2);
        if (load_factor < least_load_factor) {
            least_load_factor = load_factor;
            least_used = cur;
        }
        cur = cur->next;
    }

    return cur;
}

void shutdown_scheduler() {
    queue_destroy(tasks);
    tasks = NULL;
}
