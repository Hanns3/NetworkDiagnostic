#include "../headers/print.h"

Print::Print() {}

char* Print::get_host_name(TracerouteData& data, char* ipv4) {
    char host[NI_MAXHOST];
    struct sockaddr_in addr;
    socklen_t len;

    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_addr.s_addr = inet_addr(ipv4);
    addr.sin_family = AF_INET;
    len = sizeof(struct sockaddr_in);

    if(getnameinfo((struct sockaddr*)&addr, len, host, NI_MAXHOST, NULL, 0, NI_NAMEREQD)) {
        return ipv4;
    }
    strncpy(data.host, host, NI_MAXHOST);
    return data.host;
}

void Print::print_query(Query& query, unsigned int counter, unsigned int probe) {
    long int sec = query.receivedTime.tv_sec - query.sentTime.tv_sec;
    long int usec = query.receivedTime.tv_usec - query.sentTime.tv_usec;

    long long int total_usec = sec * 1000000 + usec;
    if(query.status != PacketStatus::SENT) {
        std::cout << total_usec/1000 << "." << total_usec%1000 <<" ms";
    }
    else std::cout << "*";
    if(counter < probe -1 ) {
        std::cout << " ";
    }
    else std::cout << std::endl;
}

void Print::sort_queries(TracerouteData& data) {
    unsigned int i = 0;
    unsigned int j = 0;
    Query temp;
    while(i < data.tqueries) {
        j = i + 1;
        while(j < data.tqueries) {
            if(data.queries[i].ttl > data.queries[j].ttl) {
                temp = data.queries[i];
                data.queries[i] = data.queries[j];
                data.queries[j] = temp;
            }
            j++;
        }
        i++;
    }
}

void Print::print_all_queries(TracerouteData& data) {
    unsigned int i = 0;
    while(i < data.tqueries) {
        if(data.queries[i].status != PacketStatus::NOT_USED) {
            std::cout << "Query: " << i << " TTL: " << data.queries[i].ttl << " Port: " << data.queries[i].port << " Status: " << (int)data.queries[i].status << std::endl;
        }
        i++;
    }
}

int Print::print_everything(TracerouteData& data) {
    unsigned int i = 0;
    sort_queries(data);
    while(i < data.tqueries) {
        if(data.queries[i].status != PacketStatus::NOT_USED 
            && data.queries[i].status != PacketStatus::DISPLAYED) {
            if(data.tprobe != data.queries[i].ttl) {
                if(data.cprobe != data.sttl)
                    std::cout << "\n";
                data.tprobe = data.queries[i].ttl;
                strncpy(data.aprobe, data.queries[i].ipv4, INET_ADDRSTRLEN);
                std::cout << data.cprobe;
                if(data.queries[i].status != PacketStatus::SENT) {
                    std::cout << " " << get_host_name(data,data.aprobe) << " (" << data.aprobe << ")";
                }
                data.cttl = 0;
                data.cprobe++;
            }
            else if(strcmp(data.aprobe, data.queries[i].ipv4) != 0) {
                strncpy(data.aprobe, data.queries[i].ipv4, INET_ADDRSTRLEN);
                std::cout << " " << get_host_name(data, data.aprobe) << " (" << data.aprobe<< ")";
            }

            if(data.cttl < data.probe) {
                print_query(data.queries[i], data.cttl, data.probe);
                data.cttl++;
            }
            data.queries[i].status = PacketStatus::DISPLAYED;

        }
        i++;
    }
    if(CURRENT_QUERY >= data.tqueries || data.reached) {
        std::cout << std::endl;
        return 1;
    }
    return 0;
}

