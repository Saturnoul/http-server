//
// Created by saturn on 4/9/21.
//
#include "server.h"

const char* server::POST = "POST";
const char* server::GET = "GET";


server& server::ip(in_addr_t ip) {
    address.sin_addr.s_addr = htonl(ip);
    return *this;
}

server& server::ip(const char* ip) {
    inet_aton(ip, &address.sin_addr);
    return *this;
}

server& server::port(const char* port) {
    address.sin_port = htons(atoi(port));
    return *this;
}

server& server::port(short port) {
    address.sin_port = htons(port);
    return *this;
}

server& server::build() {
    address.sin_family = AF_INET;
    serv_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    return *this;
}

server::server() {
    memset(&address, 0, sizeof(address));
}

void server::start() {
    int n = 1;
    setsockopt(serv_socket, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int));
    if(bind(serv_socket, (struct sockaddr*)&address, sizeof(address)) < 0){
        perror("In bind");
    }
    listen(serv_socket, 5);
    std::cout << "Listening on port " << ntohs(address.sin_port) << std::endl;

    sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    int clnt_socket = accept(serv_socket, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    HttpRequest httpRequest(clnt_socket);

    handleRequest(httpRequest, clnt_socket);

    close(clnt_socket);
    close(serv_socket);
}

void server::post(const std::string& path, const handler_type& callback) {
    setHandler("POST", path, callback);
}

void server::get(const std::string& path, const handler_type& callback) {
    setHandler("GET", path, callback);
}

void server::handleRequest(HttpRequest &request, int clnt_sock) {
    auto response = DefaultHttpResponse();
    auto method = request.getMethod();
    auto path = request.getPath();

    getHandler(method, path)(request, response);

    response.write(clnt_sock);
}

void server::setHandler(const std::string& method, const std::string& path, const handler_type& handler) {
    router[method][path] = handler;
}

handler_type& server::getHandler(const std::string &method, const std::string &path) {
    return router[method][path];
}

void server::setStaticPath(const std::string &path) {

}