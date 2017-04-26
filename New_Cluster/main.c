
int main(int argc, char const *argv[]) {
    if (argc < 3) {
        printf("USAGE: ./worker host port\n");
        return 1;
    }
    worker_main((char*)argv[1], (char*)argv[2]);
    return 0;
}
