//
// Created by saturn on 4/9/21.
//

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory.h>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <map>
#include <functional>
#include <sys/epoll.h>

#include "../message/HttpRequest.h"
#include "../message/HttpResponse.h"
#include "../util/thread_pool.h"

typedef std::function<void(const HttpRequest&, HttpResponse&)> handler_type;

class server {
public:
    server();
    server& ip(const char* ip);
    server& ip(in_addr_t ip);
    server& port(const char* port);
    server& port(short port);
    server& build();

public:
    void post(const std::string& path, const handler_type& callback);
    void get(const std::string& path, const handler_type& callback);
    void setStaticPath(const std::string& path);
    void start();

private:
    void setHandler(const std::string& method, const std::string& path, const handler_type& handler);
    handler_type& getHandler(const std::string& method, const std::string& path);
    void handleRequest(const HttpRequest& request, HttpResponse& response);
    bool isStaticResource(std::string& method, std::string& path);
    void handleStaticResource(std::string& path, HttpResponse& response);
    bool isPathMapped(std::string& method, std::string& path);
private:
    sockaddr_in address;
    int serv_socket, epfd;
    epoll_event *epoll_events;
    epoll_event event;

    ThreadPool threadPool = ThreadPool(16);


    std::map<std::string, std::map<std::string, handler_type>> mRouter;
private:
    static const char* POST;
    static const char* GET;

    friend class HttpMessage;
};

struct handler_context {
    int clnt_sock;
    server* mServer;

    handler_context(int clnt_sock, server* s) : clnt_sock(clnt_sock), mServer(s) {}
};

#endif //HTTP_SERVER_H
