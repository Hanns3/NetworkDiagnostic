#ifndef TRACEROUTE_H
#define TRACEROUTE_H

#include "traceroute_data.h"
#include "print.h"
class Print;

#define CURRENT_QUERY  data.port - data.sport

class Traceroute {
private:
    TracerouteData data;

public:
    Traceroute(const char *destination,const char *path, const char* iface, unsigned int port);
    ~Traceroute();
    friend class Print;

    void run();
    void resolve();
    int create_sockets();
    void clear_sockets();
    int monitor();
    int iterate();
    int send_packet(int rsocket);
    void receive();
    int receive_packet(int rsocket);
    int fill_query(Packet rec_packet, struct sockaddr_in* rec_addr);
    int get_packet_index(unsigned int port);
};

#endif