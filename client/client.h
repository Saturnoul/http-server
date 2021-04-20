//
// Created by saturn on 4/9/21.
//

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory.h>
#include <cstdlib>
#include <unistd.h>

class client {
public:
    client();
    client ip(const char* ip);
    client ip(long ip);
    client ip(in_addr_t ip);
    client port(const char* port);
    client port(short port);
    client build();
    void start();
private:
    sockaddr_in address;
    int clnt_socket;
};


#endif //HTTP_CLIENT_H
