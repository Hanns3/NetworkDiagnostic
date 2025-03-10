#include "../headers/traceroute.h"

Traceroute::Traceroute(char* destination, char* path) {
    data.address = destination;
    data.path = path;
    data.hops = 30;
    data.size = 32;
    data.probe = 1;
    data.ttl = 1;
    data.sttl = 1;
    data.squeries = 16;
    data.tqueries = ((data.hops - data.sttl) * data.probe);
    data.port = 33333;
    data.sport = 33333;
    
    data.timeout.tv_sec = 5;
    data.timeout.tv_usec = 0;
    data.reached = 0;
    data.sent = 0;
    data.dropped = 0;
    data.tprobe = 0;
    data.pend = 0;
    data.cprobe = 0;
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
    unsigned int i = 0;
    resolve();
    std::cout << "Traceroute to " << data.address << " (" 
    << data.ipv4 << "), " << data.hops << " hops max, " << data.size 
    << " byte packets" << std::endl;

    if(create_sockets() == -1)
        return;
    while(!(i = monitor()));
    
    clear_sockets();
}

int Traceroute::monitor() {
    if(CURRENT_QUERY < data.tqueries && 
        select(data.maxfd + 1, NULL, &data.udpfds, NULL, &data.timeout))  
    {
        if(iterate() < 0)
            return 1;
    }
    receive();
    Print print;
    std::cout << "Sent " << data.sent << " packets" <<  " CURRENT_QUERY: " << CURRENT_QUERY << std::endl;
    return print.print_everything(data);
}

int Traceroute::iterate() {
    unsigned int i = 0;
    std::cout << "Data port: " << data.port << " Data sport: " << data.sport << std::endl;
    while(i < data.squeries) {
        if(FD_ISSET(data.udp_sockets[i], &data.udpfds) && CURRENT_QUERY < data.tqueries) {
            if(send_packet(data.udp_sockets[i]) == -2)
                return -1;
            data.sent++;
            data.ttl = data.sttl + ((CURRENT_QUERY) / data.probe);
            data.port++;
        }
        i++;
    }
    return 0;
}

int Traceroute::send_packet(int rsocket) {
    char buffer[data.size];
    unsigned int i = 0;
    int frag =  IP_PMTUDISC_DONT;

    while(i < data.size) {
        buffer[i] = 0x40 + i;
        i++;
    }

    data.servaddr->sin_port = htons(data.port);
    data.servaddr->sin_family = AF_INET;
    data.host_addr = (struct sockaddr*) data.servaddr;
    
    // Checking port
    if(data.port > USHRT_MAX) {
        std::cerr << "Error: port number is too high" << std::endl;
        return -2;
    }

    // Setting TTL
    if(setsockopt(rsocket, SOL_IP, IP_TTL, &data.ttl, sizeof(data.ttl)) != 0) {
        std::cerr << "Error: unable to set TTL" << std::endl;
        return -2;
    }
    // Setting fragmentation
    if(setsockopt(rsocket, IPPROTO_IP, IP_MTU_DISCOVER, &frag, sizeof(frag)) < 0) {
        std::cerr << "Error: unable to set fragmentation" << std::endl;
        return -2;
    }
    // Sending packet
    if(sendto(rsocket, buffer, data.size, 0, data.host_addr, sizeof(struct sockaddr)) < 0) {
        std::cerr << "Error: unable to send packet" << std::endl;
        return -2;
    }
    data.queries[CURRENT_QUERY].port = data.port;
    data.queries[CURRENT_QUERY].ttl = data.ttl;
    data.queries[CURRENT_QUERY].status = PacketStatus::SENT;
    gettimeofday(&data.queries[CURRENT_QUERY].sentTime, NULL);

    return rsocket;
}

void Traceroute::receive() {
    unsigned int i = 0;
    unsigned int rec = 0;

    if(FD_ISSET(data.icmp_socket, &data.icmpfds)) {
        while (rec < data.sent)
        {
            if(receive_packet(data.icmp_socket) > 0) {
                rec++;
            }
            i++;
        }
    }
}

int Traceroute::fill_query(Packet rec_packet, struct sockaddr_in* rec_addr) {
    unsigned int port;
    uint8_t type = 0;
    struct iphdr *ip_hdr;
    struct icmphdr *icmp_hdr;
    struct udphdr *udp_hdr;
    unsigned int index;
    struct timeval end_time;

    if(gettimeofday(&end_time, NULL) < 0) {
        std::cerr << "Error: unable to set packet receiving time" << std::endl;
        return -1;
    }
    icmp_hdr = (struct icmphdr*) (&rec_packet.content.header);
    ip_hdr = (struct iphdr*) (&rec_packet.content.msg);
    udp_hdr = (struct udphdr*) ((void*) ip_hdr + sizeof(struct iphdr));
    
    port = ntohs(udp_hdr->dest);
    type = icmp_hdr->type;
    index = get_packet_index(port);
    if(index >= data.tqueries) {
        std::cerr << "Error: unable to find packet index" << std::endl;
        return -1;
    }

    strncpy(data.queries[index].ipv4, inet_ntoa(rec_addr->sin_addr), INET_ADDRSTRLEN);
    data.queries[index].receivedTime.tv_sec = end_time.tv_sec;
    data.queries[index].receivedTime.tv_usec = end_time.tv_usec;
    if(type == ICMP_TIME_EXCEEDED) {
        data.queries[index].status = PacketStatus::RECEIVED;
    } else if(type == ICMP_DEST_UNREACH) {
        data.queries[index].status = PacketStatus::RECEIVED_END;
        data.reached = 1;
    }
    return 1;
}

int Traceroute::receive_packet(int rsocket) {
    Packet rec_packet;
    struct sockaddr rec_addr;
    socklen_t rec_len = sizeof(rec_addr);
    int flags;
    if(data.dropped) {
        flags = MSG_DONTWAIT;
    } else flags = 0;

    if(setsockopt(rsocket, SOL_SOCKET, SO_RCVTIMEO, &data.timeout, sizeof(data.timeout)) < 0) {
        std::cerr << "Error: unable to set receiver's timeout" << std::endl;
        return -1;
    }

    memset(&rec_packet, 0, sizeof(rec_packet));
    memset(&rec_addr, 0, sizeof(rec_addr));

    if(recvfrom(rsocket, &rec_packet, sizeof(rec_packet), flags, &rec_addr, &rec_len) <= 0) {
        std::cerr << "Error: unable to receive packet" << std::endl;
        data.dropped = 1;
        return 1;
    }
    return fill_query(rec_packet, (struct sockaddr_in*) &rec_addr);
}



int Traceroute::get_packet_index(int port) {
    unsigned int i = 0;
    while(i < data.tqueries) {
        if(data.queries[i].port == port) {
            return i;
        }
        i++;
    }
    return i;
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

void Traceroute::clear_sockets() {
    unsigned int i = 0;
    while(i < data.squeries) {
        FD_CLR(data.udp_sockets[i], &data.udpfds);
        close(data.udp_sockets[i]);
        i++;
    }
    FD_CLR(data.icmp_socket, &data.icmpfds);
    close(data.icmp_socket);
}

Traceroute::~Traceroute() {
    freeaddrinfo(data.host_info);
}