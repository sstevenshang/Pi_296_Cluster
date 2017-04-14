#pragma once
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_
#include "queue.h"
#include "node.h"

void schedule_task(char* task);
void distribute_task(node* workers);
void recover_tasks(node* dead_worker);
//Returns the worker node that is least used
node* get_least_used_worker();

#endif
