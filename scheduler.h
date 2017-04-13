#pragma once
#include <queue.h>
#include <node.h>

queue* tasks;

void schedule_task(char* task);
void distribute_task(node* workers);
