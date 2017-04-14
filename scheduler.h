#pragma once
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_
#include "queue.h"
#include "node.h"
#include <unistd.h>

void schedule_task(task* work);
void get_task_from_queue_onto_nodes(node* workers);
int distribute_task(node* workers, task* work);
void recover_tasks(node* worker);
node* get_least_used_worker(node* workers);
void shutdown_scheduler();

#endif
