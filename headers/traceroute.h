#ifndef TRACEROUTE_H
#define TRACEROUTE_H

#include "traceroute_data.h"


#define CURRENT_QUERY  data.port - data.sport

class Traceroute {
private:
    TracerouteData data;

public:
    Traceroute(char *destination, char *path);
    ~Traceroute();

    void run();
    void resolve();
    int create_sockets();
    void clear_sockets();
    void monitor();
    int iterate();
    int send_packet(int rsocket);
    void receive();
    int receive_packet(int rsocket);
    int fill_query(Packet rec_packet, struct sockaddr_in* rec_addr);
    int get_packet_index(int port);
    void print_packet_data(const void* data, size_t length);

};

#endif