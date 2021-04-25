//
// Created by saturn on 4/9/21.
//
#include "server.h"
#include <fstream>


const char *server::POST = "POST";
const char *server::GET = "GET";

const int READ_BUF_SIZE = 1024;
const int EPOLL_SIZE = 50;


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

server &server::build() {
    address.sin_family = AF_INET;
//    serv_socket = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    serv_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    return *this;
}

server::server() {
    memset(&address, 0, sizeof(address));
    epfd = epoll_create(EPOLL_SIZE);
    epoll_events = static_cast<epoll_event *>(malloc(sizeof(epoll_event) * EPOLL_SIZE));
}

void server::start() {
    int n = 1, clnt_sock = -1;
    sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    int event_cnt = 0;

    setsockopt(serv_socket, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int));
    if (bind(serv_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("In bind");
    }
    listen(serv_socket, 1000);
    std::cout << "Listening on port " << ntohs(address.sin_port) << std::endl;

//    event.events = EPOLLIN;
//    event.data.fd = serv_socket;
//    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_socket, &event);
//
//    while (true) {
//        event_cnt = epoll_wait(epfd, epoll_events, EPOLL_SIZE, -1);
//        if (event_cnt == -1) {
//            break;
//        }
//        for (int i = 0; i < event_cnt; i++) {
//            if (epoll_events[i].data.fd == serv_socket) {
//                clnt_sock = accept(serv_socket, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
//                event.events = EPOLLIN | EPOLLET;
//                event.data.fd = clnt_sock;
//                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
//            } else {
//                clnt_sock = epoll_events[i].data.fd;
//                threadPool.enqueue([this, clnt_sock] {
//                    HttpRequest request(clnt_sock);
//                    DefaultHttpResponse response(clnt_sock);
//                    handleRequest(request, response);
//                    epoll_ctl(epfd, EPOLL_CTL_DEL, clnt_sock, &event);
//                });
//            }
//        }
//    }
//    close(epfd);

    while ((clnt_sock = accept(serv_socket, (struct sockaddr *) &clnt_addr, &clnt_addr_size)) != -1) {
//        std::cout << inet_ntoa(clnt_addr.sin_addr) << std::endl;
        threadPool.enqueue([this, clnt_sock] {
            HttpRequest request(clnt_sock);
            DefaultHttpResponse response(clnt_sock);
            handleRequest(request, response);
        });
    };
    close(serv_socket);
}

void server::post(const std::string &path, const handler_type &callback) {
    setHandler("POST", path, callback);
}

void server::get(const std::string &path, const handler_type &callback) {
    setHandler("GET", path, callback);
}

void server::handleRequest(const HttpRequest &request, HttpResponse &response) {
    auto method = request.getMethod();
    auto path = request.getPath();

    if (isStaticResource(method, path)) {
        handleStaticResource(path, response);
    } else {
        if (isPathMapped(method, path)) {
            getHandler(method, path)(request, response);
        } else {
            response.setStatusCode(404);
        }
        response.send();
        response.end();
    }
}

void server::setHandler(const std::string &method, const std::string &path, const handler_type &handler) {
    mRouter[method][path] = handler;
}

handler_type &server::getHandler(const std::string &method, const std::string &path) {
    return mRouter[method][path];
}

void server::setStaticPath(const std::string &path) {
    resource::init(path);
}

bool server::isStaticResource(std::string &method, std::string &path) {
    if (method == GET && resource::getInstance()->isStaticResource(path)) {
        return true;
    }
    return false;
}

void server::handleStaticResource(std::string &path, HttpResponse &response) {
    std::fstream fs;
    char buf[READ_BUF_SIZE];
    int left = 0;

    fs.open(resource::getInstance()->getFullPath(path), std::ios::in | std::ios::binary);
    response.directoryWriteHeader();
    while (fs.read(buf, READ_BUF_SIZE)) {
        response.directWriteBody(buf, READ_BUF_SIZE);
    }
    left = fs.gcount();
    fs.read(buf, left);
    response.directWriteBody(buf, left);

    response.end();
    fs.close();
}

bool server::isPathMapped(std::string &method, std::string &path) {
    return mRouter[method].find(path) != mRouter[method].end();
}