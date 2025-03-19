#ifndef TRACEROUTE_DATA_H
#define TRACEROUTE_DATA_H

#include <iostream>
#include <cstring>
#include <vector>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <limits.h>
#include <unistd.h>
#include <iomanip>


enum class PacketStatus {
    NOT_USED,
    SENT,
    RECEIVED,
    RECEIVED_END,
    DISPLAYED
};


class Query {
public:
    char ipv4[INET_ADDRSTRLEN];
    unsigned int ttl;
    unsigned int port;
    struct timeval sentTime;
    struct timeval receivedTime;
    PacketStatus status;
};

class TracerouteData {
public:
    std::string path;
    std::string interface;
    char ipv4[INET_ADDRSTRLEN];
    struct addrinfo *host_info;
    struct sockaddr* host_addr;
    struct sockaddr_in* servaddr;
    struct timeval start_time;
    struct timeval end_time;
    std::string address;

    unsigned int hops; // Number of max hops
    unsigned int size; // Size of the packet
    unsigned int squeries; // Number of simultaneous queries
    unsigned int tqueries; // Number of total queries
    unsigned int ttl; // Time to live
    unsigned int sttl;
    unsigned int probe;
    unsigned int port;
    unsigned int sport;
    unsigned int sent;
    struct timeval timeout;
    uint8_t reached;
    uint8_t dropped;

    fd_set udpfds;
    fd_set icmpfds;
    int maxfd;
    std::vector<int> udp_sockets;
    int icmp_socket;

    std::vector<Query> queries; // Information about the queries
    char aprobe[INET_ADDRSTRLEN]; // Address of the probe
    unsigned int tprobe; // TTL of current probe
    unsigned int cprobe; // Probe counter
    uint8_t pend; // Flag to finish the traceroute
    char host[NI_MAXHOST]; // Hostname of current probe
    unsigned int cttl;  // Packet count for current TTL
    
};

class ICMPPacket {
public:
    struct icmphdr header;
    char msg[60 - sizeof(struct icmphdr)];
};

class UDPPacket {
public:
    struct udphdr header;
    char msg[60 - sizeof(struct udphdr)];

};

class Packet {
public:
    struct iphdr ip_header;
    ICMPPacket content;

};

#endif