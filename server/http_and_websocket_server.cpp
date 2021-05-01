//
// Created by saturn on 4/29/21.
//


#include <iostream>
#include "http_and_websocket_server.h"

const unsigned char C_40 = 0x40;

void http_and_websocket_server::start_with_custom() {
    int clnt_sock = -1;
    sockaddr_in clnt_addr{};
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    unsigned char first_byte = 0;

    int epfd = epoll_create(EPOLL_SIZE);
    auto* epoll_events = static_cast<epoll_event *>(malloc(sizeof(epoll_event) * EPOLL_SIZE));

    ThreadPool threadPool(THREAD_POOL_SIZE);
    int event_cnt = 0;
    epoll_event event{};

    event.events = EPOLLIN | EPOLLET;
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
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
            } else {
                clnt_sock = epoll_events[i].data.fd;
                sock_reader sr(clnt_sock);
                sr.peek(&first_byte, 1);
                if(first_byte & C_40){  // Since we don't support websocket extensions, the second bit of the first byte in a frame must be 0.
                    HttpRequest request(sr);
                    auto req_header = dynamic_cast<const request_header*>(request.getAllHeader());
                    if(WebsocketHandshake::verifyRequest(req_header)){

                        WebsocketHandshake handshake(request, clnt_sock);
                        handleHandshake(handshake);
                    }else {
                        threadPool.enqueue([this, clnt_sock](const HttpRequest& request) {
                            handleHttpRequest(request, clnt_sock);
                        }, std::move(request));
                    }
                }else{
                    handleFrame(sr);
                }
            }
        }
    }
    close(epfd);
    close(serv_socket);
    delete epoll_events;
}
