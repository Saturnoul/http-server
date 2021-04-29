//
// Created by saturn on 4/9/21.
//

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <arpa/inet.h>
#include <sys/epoll.h>
#include "../thirdparty/thread_pool.h"

const int THREAD_POOL_SIZE = 16;

class server {
public:
    server();
    virtual ~server()=default;

public:
    server& ip(const char* ip);
    server& ip(in_addr_t ip);
    server& port(const char* port);
    server& port(short port);
    server& build();
    void start_with_thread_pool();
    void start_with_epoll();

protected:
    virtual void handle_connection(int clnt_sock, bool initial) = 0;
protected:
    sockaddr_in address;
    int serv_socket;

    ThreadPool threadPool = ThreadPool(THREAD_POOL_SIZE);
};

#endif //HTTP_SERVER_H
