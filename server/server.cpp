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
    listen(serv_socket, SERVER_WATING_SIZE);
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

connection::connection(int clnt_sock, int epfd) : clnt_sock(clnt_sock), mEpollFd(epfd){
    mLen = 0;
    mWrittenLen = 0;
    mDataComplete = false;
}

bool connection::writeData() {
    int writeLen = ::send(clnt_sock, mData + mWrittenLen, mLen - mWrittenLen, MSG_DONTWAIT);
    if(writeLen > 0) {
        mWrittenLen += writeLen;
    }
    return mWrittenLen == mLen;
}

bool connection::write() {
    if(mDataComplete && mFp) {
        return writeFile();
    }
    if((mDataComplete = writeData())) {
        mLen = mWrittenLen = 0;
    }
    return mDataComplete && !mFp;
}

void connection::setData(const char* data, int len) {
    mData = data;
    mLen = len;
    listenForOut();
}

void connection::listenForOut() const {
    epoll_event event {};
    event.events = EPOLLOUT;
    event.data.fd = clnt_sock;
    epoll_ctl(mEpollFd, EPOLL_CTL_MOD, clnt_sock, &event);
}

void connection::setFile(FILE *fp) {
    mFp = fp;
}

bool connection::writeFile() {
    if(mLen == 0) {
        mLen = fread(mBuf, 1, SERVER_BUF_SIZE, mFp);
        if(mLen <= 0) {
            return true;
        }
    }
    int writeLen = ::send(clnt_sock, mBuf + mWrittenLen, mLen - mWrittenLen, MSG_DONTWAIT);
    if(writeLen > 0){
        mWrittenLen += writeLen;
    }
    if(mWrittenLen == mLen) {
        mLen = mWrittenLen = 0;
    }
    return false;
}

connection::~connection() {
    delete mRequest;
    close(clnt_sock);
    mFp && fclose(mFp);
}

