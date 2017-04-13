#pragma once
#include "queue.h"
#include "node.h"

void schedule_task(char* task);
void distribute_task(node* workers);
void recover_tasks(node* worker);
//Returns the worker node that is least used
node* get_least_used_worker();
