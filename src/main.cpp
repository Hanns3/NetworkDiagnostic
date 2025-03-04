#include "../headers/traceroute.h"

int main(int argc, char *argv[]) {
    Traceroute traceroute(argv[1], argv[0]);
    traceroute.run();
    return 0;
}