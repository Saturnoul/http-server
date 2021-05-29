//
// Created by saturn on 4/9/21.
//

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <map>
#include <unistd.h>
#include <atomic>
#include <iostream>
#include <fcntl.h>
#include "../thirdparty/thread_pool.h"

const int THREAD_POOL_SIZE = 16;
const int EPOLL_TIMEOUT = 500;
const int EPOLL_SIZE = 100;
const int SERVER_BUF_SIZE = 1024;

class server {
public:
    server();
    virtual ~server()=default;

public:
    server& ip(const char* ip);
    server& ip(in_addr_t ip);
    server& port(const char* port);
    server& port(short port);
    void build();
    void start_with_thread_pool();

    virtual void start_with_custom();

protected:
    virtual void handle_connection_thread(int clnt_sock);
protected:
    sockaddr_in address;
    int serv_socket;
};


class connection {
public:
    connection(int clnt_sock, int epfd);
public:
    bool operator<(const connection& conn) const;
    bool ready() const;
    void setData(const char* data, int len);
    void writeToClient();
    void readFromClient(server* pServer);
    void clear() const;
    bool writeFile();

public:
    void setRemoveFunc(const std::function<void(int)>& func) {
        removeSelfConnection = func;
    }
    void setFile(FILE* fp) {
        mFp = fp;
    }
public:
    virtual bool read(server* srv);
    virtual ~connection();

private:
    void removeSelfListening() const;
    void listenForOut() const;

protected:
    int clnt_sock;
    int mEpollFd;
    std::atomic_bool mReadyToWrite;
    const char* mData;
    FILE* mFp;
    int mLen;
    int mWrittenLen;
    void* mRequest;
    char mBuf[SERVER_BUF_SIZE];
private:
    std::function<void(int clnt_sock)> removeSelfConnection;
};


template<typename CONNECTION_TYPE>
class nonblocking_server : public server {
public:
    void start_with_epoll() {
        int clnt_sock = -1;
        sockaddr_in clnt_addr{};
        socklen_t clnt_addr_size = sizeof(clnt_addr);

        mEpollFd = epoll_create(EPOLL_SIZE);
        auto* epoll_events = static_cast<epoll_event *>(malloc(sizeof(epoll_event) * EPOLL_SIZE));

        int event_cnt = 0;
        epoll_event event{};

        event.events = EPOLLIN;
        event.data.fd = serv_socket;
        epoll_ctl(mEpollFd, EPOLL_CTL_ADD, serv_socket, &event);

        while (true) {
            event_cnt = epoll_wait(mEpollFd, epoll_events, EPOLL_SIZE, EPOLL_TIMEOUT);
//        if (event_cnt == -1) {
//            break;
//        }
            for (int i = 0; i < event_cnt; i++) {
                auto event = epoll_events[i];
                if (event.data.fd == serv_socket) {
                    clnt_sock = accept(serv_socket, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
                    SetBlock(clnt_sock, false);
                    event.events = EPOLLIN | EPOLLET;
                    event.data.fd = clnt_sock;
                    epoll_ctl(mEpollFd, EPOLL_CTL_ADD, clnt_sock, &event);
                } else {
                    clnt_sock = event.data.fd;
                    handle_connection_epoll(clnt_sock, event.events);
                }
            }
        }
        close(mEpollFd);
        close(serv_socket);
        delete epoll_events;
    }
public:
    void addConnection(int clnt_sock, connection* conn) {
        CONNECTION_MAP.insert(std::pair<int, connection*>(clnt_sock, conn));
    }
    void removeConnection(int clnt_sock) {
        auto conn_pair = CONNECTION_MAP.find(clnt_sock);
        if(conn_pair != CONNECTION_MAP.end()) {
            CONNECTION_MAP.erase(conn_pair);
            delete conn_pair->second;
        }
    }

private:
    void handle_connection_epoll(int clnt_sock, uint32_t events) {
        connection* conn;
        auto conn_pair = CONNECTION_MAP.find(clnt_sock);
        if(conn_pair == CONNECTION_MAP.end()){
            conn = new CONNECTION_TYPE(clnt_sock, mEpollFd);
            conn->setRemoveFunc([this](int clnt_sock) {
                removeConnection(clnt_sock);
            });
            addConnection(clnt_sock, conn);
        } else {
            conn = conn_pair->second;
        }
        if(events & EPOLLIN) {
            conn->readFromClient(this);
        }
        else if(events & EPOLLOUT && conn->ready()) {
            conn->writeToClient();
        }
    }
    static bool SetBlock(int sock, bool isBlock) {
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
        if (isBlock) {
            flags = flags & ~O_NONBLOCK;
        } else {
            flags = flags | O_NONBLOCK;
        }
        re = fcntl(sock, F_SETFL, flags);
#endif
        if (re != 0) return false;
        return true;
    }

private:
    std::map<int, connection*> CONNECTION_MAP;
    int mEpollFd = -1;
};

#endif //HTTP_SERVER_H
