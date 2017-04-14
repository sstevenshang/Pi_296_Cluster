#include "scheduler.h"

queue* tasks;

//TODO let's squitch over for a task struct for clarity. We can store filenames + status of a task. See node.h.

void schedule_task(task* work) {
    if (tasks == NULL)
        tasks = queue_create();
    queue_push(tasks, work);
}

void get_task_from_queue_onto_nodes(node* workers) {
    while (!queue_empty(tasks)) {
        task* work = queue_pull(tasks);
        if (distribute_task(workers, work) == 0) {
            // TODO: figure out a better way
            usleep(0.2*1000);
        }
    }
}

int distribute_task(node* workers, task* work) {
  node* the_one = get_least_used_worker(workers);
  if (the_one == NULL) {
      queue_push(tasks, work);
      return 0;
  } else {
      for (size_t i=0; i<5; i++) {
         if (the_one->task_list[i] == NULL) {
             the_one->task_list[i] = work;
             set_num_of_task(the_one, get_num_of_task(the_one)+1);
             return 1;
         }
      }
  }
  return 0;
}

//Put any tasks the woker was working on back onto the queue
//"Recover" the tasks
void recover_tasks(node* worker) {
    task** cur = worker->task_list;
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
        if (is_alive(cur)) {
            continue;
        }
        int num = get_num_of_task(cur);
        if (num >= 5) {
            continue;
        }
        load_factor = (cur->cur_load) + (num * 2);
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
