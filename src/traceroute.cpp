#include "../headers/traceroute.h"

Traceroute::Traceroute(char* destination, char* path) {
    data.address = destination;
    data.path = path;
    data.hops = 30;
    data.size = 32;
    data.probe = 3;
    data.ttl = 1;
    data.sttl = 1;
    data.squeries = 16;
    data.tqueries = ((data.hops - data.sttl) * data.probe);
    data.port = 33434;
    data.sport = 33434;
    data.sent = 0;
    data.timeout.tv_sec = 5;
    data.timeout.tv_usec = 0;
    data.reached = 0;
    data.dropped = 0;
    data.tprobe = 0;
    data.pend = 0;
    data.udp_sockets.resize(data.squeries);
    data.queries.resize(data.tqueries);
    FD_ZERO(&data.udpfds);
    FD_ZERO(&data.icmpfds);
    data.maxfd = 0;
    data.icmp_socket = 0;
    data.host_info = NULL;
}

void Traceroute::resolve() {
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    std::cout << "Resolving hostname " << data.address << std::endl;
    if(getaddrinfo(data.address.c_str(), NULL, &hints, &data.host_info) != 0 || 
        data.host_info == NULL) {
        std::cerr << "Error: unable to resolve hostname" << std::endl;
        exit(1);
    }

    data.host_addr = data.host_info->ai_addr;
    data.servaddr = (struct sockaddr_in*) data.host_addr;
    inet_ntop(AF_INET, &(data.servaddr->sin_addr), data.ipv4, INET_ADDRSTRLEN);
}

void Traceroute::run() {
    resolve();
    std::cout << "Traceroute to " << data.address << " (" 
    << data.ipv4 << "), " << data.hops << " hops max, " << data.size 
    << " byte packets" << std::endl;
    std::cout << "1\t" << data.ipv4 << std::endl;

    if(create_sockets() == -1)
        exit(1);

}

int Traceroute::create_sockets() {
    unsigned int i = 0;
    // Create UDP sockets
    while(i < data.squeries) {
        data.udp_sockets[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(data.udp_sockets[i] == -1) {
            std::cerr << "Error: unable to create sender's socket" << std::endl;
            return -1;
        }
        FD_SET(data.udp_sockets[i], &data.udpfds);
        if(data.udp_sockets[i] > data.maxfd)
            data.maxfd = data.udp_sockets[i];
        i++;
    }
    // Create ICMP socket (works only with sudo)
    if((data.icmp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        std::cerr << "Error: unable to create receiver's socket" << std::endl;
        return -1;
    }
    FD_SET(data.icmp_socket, &data.icmpfds);
    if(data.icmp_socket > data.maxfd)
        data.maxfd = data.icmp_socket;
    return 0;
}

Traceroute::~Traceroute() {
    freeaddrinfo(data.host_info);
}