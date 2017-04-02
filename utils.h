#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/*
* Gets the current local CPU usage of the device.
* Returns -1 on failure and a percentage upon success
*/
double get_local_usage();

#endif
