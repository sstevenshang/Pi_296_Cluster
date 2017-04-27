#include "master.h"
#include "worker.h"
#include "interface.h"

//Run with just the port to run master, run with with nothing to run worker, run with IP_address_of_master and executable file name for interface
int main(int argc, char const *argv[]) {

    if (argc ==  2)
      master_main(argc, (char**)argv);
    else if (argc == 1)
      worker_main("127.0.0.1", "9999");
    else
      interface_main(argc, (char**)argv);

    return 0;
}
