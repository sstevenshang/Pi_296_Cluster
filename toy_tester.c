#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
  double d = get_local_usage();
  printf("CPU usage is %f\n", d);
  return 0;
}
