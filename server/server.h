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
#include "../thirdparty/thread_pool.h"

const int THREAD_POOL_SIZE = 16;
const int EPOLL_TIMEOUT = 500;
const int EPOLL_SIZE = 100;
const int SERVER_BUF_SIZE = 1024;

struct epoll_connection_flag {
    bool listen_for_out;
    bool remove;
};

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
    void start_with_epoll();

    virtual void start_with_custom();

protected:
    virtual void handle_connection_thread(int clnt_sock);
    virtual void handle_connection_epoll(int clnt_sock, uint32_t events, epoll_connection_flag& flag);
protected:
    sockaddr_in address;
    int serv_socket;

private:
    static bool SetBlock(int sock, bool isBlock);
};


class connection {
public:
    connection(int clnt_sock);
public:
    bool operator<(const connection& conn) const;
    bool ready() const;
    void setData(const char* data, int len);
    bool write();
    bool writeFile();

public:
    void setFile(FILE* fp) {
        mFp = fp;
    }
public:
    virtual bool read(server* srv);
    virtual ~connection();

protected:
    int clnt_sock;
    std::atomic_bool mReadyToWrite;
    const char* mData;
    FILE* mFp;
    int mLen;
    int mWrittenLen;
    void* mRequest;
    char mBuf[SERVER_BUF_SIZE];
};


template<typename CONNECTION_TYPE>
class nonblocking_server : public server {
public:
    void addConnection(int clnt_sock, CONNECTION_TYPE* conn) {
        CONNECTION_MAP.insert(std::pair<int, CONNECTION_TYPE*>(clnt_sock, conn));
    }
    void removeConnection(int clnt_sock) {
        auto conn_pair = CONNECTION_MAP.find(clnt_sock);
        if(conn_pair != CONNECTION_MAP.end()) {
            CONNECTION_MAP.erase(conn_pair);
            delete conn_pair->second;
        }
    }

private:
    void handle_connection_epoll(int clnt_sock, uint32_t events, epoll_connection_flag& flag) override {
        CONNECTION_TYPE* conn;
        auto conn_pair = CONNECTION_MAP.find(clnt_sock);
        std::cout << "Handle_epoll" << std::endl;
        if(conn_pair == CONNECTION_MAP.end()){
            conn = new CONNECTION_TYPE(clnt_sock);
            addConnection(clnt_sock, conn);
        } else {
            conn = conn_pair->second;
        }
        if(events & EPOLLIN) {
            std::cout << "Read: " << std::endl;
            flag.listen_for_out = conn->read(this);
        }
        else if(events & EPOLLOUT && conn->ready()) {
            std::cout << "Write: " << clnt_sock << std::endl;
            if((flag.remove = conn->write())) {
                removeConnection(clnt_sock);
            }
        }
    }

protected:
    std::map<int, CONNECTION_TYPE*> CONNECTION_MAP;
};

#endif //HTTP_SERVER_H
