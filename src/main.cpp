#include "../headers/traceroute.h"
#include <unistd.h>
#include <string>

struct Args {
    std::string interface = "eth0";
    std::string destination;
    int port = 33434;
};

Args parse_arguments(int argc, char* argv[]) {
    Args args;
    int opt;
    while((opt = getopt(argc, argv, "I:p:")) != -1) {
        switch(opt) {
            case 'I':
                args.interface = optarg;
                break;
            case 'p':
                if(std::stoi(optarg) < 1 || std::stoi(optarg) > USHRT_MAX) {
                    std::cerr << "Error: port number is too high" << std::endl;
                    exit(1);
                }
                args.port = std::stoi(optarg);
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-I interface | -p port number] destination" << std::endl;
                exit(1);
        }
    }
    if(optind < argc) {
        args.destination = argv[optind];
    }
    else {
        std::cerr << "Usage: " << argv[0] << " [-I interface | -p port number] destination" << std::endl;
        exit(1);
    }
    return args;
}

int main(int argc, char *argv[]) {
    Args args = parse_arguments(argc, argv);
    Traceroute traceroute(argv[0], args.destination.c_str(), args.interface.c_str(), args.port);
    traceroute.run();
    return 0;
}