#ifndef PRINT_H
#define PRINT_H

#include "traceroute.h"

class Print {
    public:
    Print();
    int print_everything(TracerouteData& data);
    char* get_host_name(TracerouteData& data, char* ipv4);
    void print_query(Query& query, unsigned int counter, unsigned int probe);
    void sort_queries(TracerouteData& data);
    void print_all_queries(TracerouteData& data);
};

#endif