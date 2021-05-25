//
// Created by saturn on 4/9/21.
//
#include "server.h"

#include <memory.h>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <fcntl.h>


server &server::ip(in_addr_t ip) {
    address.sin_addr.s_addr = htonl(ip);
    return *this;
}

server &server::ip(const char *ip) {
    inet_aton(ip, &address.sin_addr);
    return *this;
}

server &server::port(const char *port) {
    address.sin_port = htons(atoi(port));
    return *this;
}

server &server::port(short port) {
    address.sin_port = htons(port);
    return *this;
}

void server::build() {
    int n = 1;
    address.sin_family = AF_INET;
//    serv_socket = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    serv_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(serv_socket, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int));
    if (bind(serv_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("In bind");
    }
    listen(serv_socket, 1000);
    std::cout << "Listening on port " << ntohs(address.sin_port) << std::endl;
}

server::server() {
    memset(&address, 0, sizeof(address));
}

void server::start_with_epoll() {
    int clnt_sock = -1;
    sockaddr_in clnt_addr{};
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    int epfd = epoll_create(EPOLL_SIZE);
    auto* epoll_events = static_cast<epoll_event *>(malloc(sizeof(epoll_event) * EPOLL_SIZE));

    int event_cnt = 0;
    epoll_event event{};

    event.events = EPOLLIN;
    event.data.fd = serv_socket;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_socket, &event);

    while (true) {
        event_cnt = epoll_wait(epfd, epoll_events, EPOLL_SIZE, EPOLL_TIMEOUT);
        if (event_cnt == -1) {
            break;
        }
        for (int i = 0; i < event_cnt; i++) {
            if (epoll_events[i].data.fd == serv_socket) {
                clnt_sock = accept(serv_socket, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
                SetBlock(clnt_sock, false);
                event.events = EPOLLIN;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
            } else {
                clnt_sock = epoll_events[i].data.fd;
                if(!handle_connection_epoll(clnt_sock)) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, clnt_sock, &event);
                }
            }
        }
    }
    close(epfd);
    close(serv_socket);
    delete epoll_events;
}

void server::start_with_thread_pool() {
    int clnt_sock = -1;
    sockaddr_in clnt_addr{};
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    ThreadPool threadPool = ThreadPool(THREAD_POOL_SIZE);

    while ((clnt_sock = accept(serv_socket, (struct sockaddr *) &clnt_addr, &clnt_addr_size)) != -1) {
//        std::cout << inet_ntoa(clnt_addr.sin_addr) << std::endl;
        threadPool.enqueue([this, clnt_sock](){
            handle_connection_thread(clnt_sock);
        });
    }
    close(serv_socket);
}

void server::start_with_custom() {
    std::cout << "You need to implement your custom starter" << std::endl;
}

bool server::handle_connection_epoll(int clnt_sock) {
    std::cout << "You need to implement your starter" << std::endl;
    return false;
}

void server::handle_connection_thread(int clnt_sock) {
    std::cout << "You need to implement your starter" << std::endl;
}

bool server::SetBlock(int sock,bool isblock)
{
    int re = 0;
//通过宏区分windows和linux，如果是windows64位程序判断 _WIN64宏
#ifdef WIN32
    unsigned long ul = 0;
	if(!isblock) ul = 1;
	re = ioctlsocket(sock, FIONBIO, (unsigned long*)&ul);
#else
    //先取到现有描述符属性，保证本次更改不变动原有属性
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) {
        return false;
    }
    if(isblock)
    {
        flags = flags & ~O_NONBLOCK;
    }
    else
    {
        flags = flags | O_NONBLOCK;
    }
    re = fcntl(sock, F_SETFL, flags);
#endif
    if (re != 0) return false;
    return true;
}


bool connection::operator<(const connection &conn) const {
    return this->clnt_sock < conn.clnt_sock;
}

bool connection::read(server* srv) {
    return false;
}

connection::connection(int clnt_sock) : clnt_sock(clnt_sock){
}

connection::~connection() {
    delete mRequest;
}
