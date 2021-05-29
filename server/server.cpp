//
// Created by saturn on 4/9/21.
//
#include "server.h"

#include <memory.h>
#include <cstdlib>
#include <iostream>
#include <sys/socket.h>


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

void server::start_with_thread_pool() {
    int clnt_sock = -1;
    sockaddr_in clnt_addr{};
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    ThreadPool threadPool(THREAD_POOL_SIZE);

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

void server::handle_connection_thread(int clnt_sock) {
    std::cout << "You need to implement your starter" << std::endl;
}


bool connection::operator<(const connection &conn) const {
    return this->clnt_sock < conn.clnt_sock;
}

bool connection::read(server* srv) {
    return false;
}

connection::connection(int clnt_sock, int epfd) : clnt_sock(clnt_sock), mEpollFd(epfd){
    mRequest = nullptr;
    mReadyToWrite = false;
    mData = nullptr;
    mFp = nullptr;
    mLen = 0;
    mWrittenLen = 0;
}

void connection::writeToClient() {
    int writeLen = ::send(clnt_sock, mData + mWrittenLen, mLen - mWrittenLen, MSG_DONTWAIT);
    if(writeLen > 0) {
        mWrittenLen += writeLen;
    }
    if(mWrittenLen == mLen) {
        clear();
    }
}

bool connection::ready() const {
    return mReadyToWrite;
}

void connection::readFromClient(server* pServer) {
    if(read(pServer)) {
        listenForOut();
    }
}

void connection::clear() const {
    removeSelfListening();
    removeSelfConnection(clnt_sock);
}

void connection::setData(const char* data, int len) {
    mData = data;
    mLen = len;
    writeToClient();
    mReadyToWrite.store(true, std::memory_order_seq_cst);
}

void connection::removeSelfListening() const {
    epoll_event event {};
    epoll_ctl(mEpollFd, EPOLL_CTL_DEL, clnt_sock, &event);
}

void connection::listenForOut() const {
    epoll_event event {};
    event.events = EPOLLOUT | EPOLLET;
    event.data.fd = clnt_sock;
    epoll_ctl(mEpollFd, EPOLL_CTL_MOD, clnt_sock, &event);
}

bool connection::writeFile() {

}

connection::~connection() {
    close(clnt_sock);
    delete mRequest;
    mFp && fclose(mFp);
}
