#ifndef TRACEROUTE_H
#define TRACEROUTE_H

#include "traceroute_data.h"

class Traceroute {
private:
    TracerouteData data;

public:
    Traceroute(char *destination, char *path);
    ~Traceroute();

    void run();
    void resolve();
    int create_sockets();
};

#endif