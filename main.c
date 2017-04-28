#include "master.h"
#include "worker.h"
#include "interface.h"

int main(int argc, char const *argv[]) {
    if (argc != 2) {
      printf("Usage: \n./key_lime_cluster master\n./key_lime_cluster worker\n./key_lime_cluster interface\n");
      exit(1);
    }
    if (strcmp("master", argv[1])==0)
      master_main();
    else if (strcmp("worker", argv[1])==0)
      worker_main("127.0.0.1", "9999");
    else
      interface_main();

    return 0;
}
