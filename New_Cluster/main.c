#include "master.h"
#include "worker.h"
#include "interface.h"

int main(int argc, char const *argv[]) {

    master_main(argc, argv);

    // interface_main(argc, argv);        UNCOMMENT TO DEBUG YOUR CODE
    // worker_main("127.0.0.1", "9999");

    return 0;
}
