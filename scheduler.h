#pragma once
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_
#include "queue.h"
#include "node.h"
#include "networkManager.h"
#include <unistd.h>

void schedule_task(task* work);
void get_task_from_queue_onto_nodes(node* workers);
int distribute_task(node* workers, task* work);
void recover_tasks(node* worker);
node* get_least_used_worker(node* workers);
void remove_tasks(node* worker, task* work);
void shutdown_scheduler();
void scheduler(node* workerList, int fd);
#endif
