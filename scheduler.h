#pragma once
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_
#include "queue.h"
#include "node.h"

void schedule_task(task* task);
void distribute_task(node* workers);
void recover_tasks(node* worker);
node* get_least_used_worker(node* head_worker);
void shutdown_scheduler();

#endif
