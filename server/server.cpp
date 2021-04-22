//
// Created by saturn on 4/9/21.
//
#include "server.h"
#include <fstream>
#include <pthread.h>

const char* server::POST = "POST";
const char* server::GET = "GET";

const int READ_BUF_SIZE = 1024;


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
    int n = 1, clnt_sock = -1;
    sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    pthread_t t_id;

    setsockopt(serv_socket, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int));
    if(bind(serv_socket, (struct sockaddr*)&address, sizeof(address)) < 0){
        perror("In bind");
    }
    listen(serv_socket, 5);
    std::cout << "Listening on port " << ntohs(address.sin_port) << std::endl;


    while ((clnt_sock = accept(serv_socket, (struct sockaddr*)&clnt_addr, &clnt_addr_size)) != -1){
        auto context = new handler_context(clnt_sock, this);
        pthread_create(&t_id, NULL, &handleClient, context);
        pthread_detach(t_id);
    }
    close(serv_socket);
}

void server::post(const std::string& path, const handler_type& callback) {
    setHandler("POST", path, callback);
}

void server::get(const std::string& path, const handler_type& callback) {
    setHandler("GET", path, callback);
}

void server::handleRequest(const HttpRequest &request, HttpResponse& response) {
    auto method = request.getMethod();
    auto path = request.getPath();

    if(isStaticResource(method, path)){
        handleStaticResource(path, response);
    } else{
        if(isPathMapped(method, path)){
            getHandler(method, path)(request, response);
        } else{
            response.setStatusCode(404);
        }
        response.send();
    }
}

void server::setHandler(const std::string& method, const std::string& path, const handler_type& handler) {
    mRouter[method][path] = handler;
}

handler_type& server::getHandler(const std::string &method, const std::string &path) {
    return mRouter[method][path];
}

void server::setStaticPath(const std::string &path) {
    resource::init(path);
}

bool server::isStaticResource(std::string &method, std::string &path) {
    if(method == GET && resource::getInstance()->isStaticResource(path)){
        return true;
    }
    return false;
}

void server::handleStaticResource(std::string& path, HttpResponse& response) {
    std::fstream fs;
    char buf[READ_BUF_SIZE];
    int left = 0;

    fs.open(resource::getInstance()->getFullPath(path), std::ios::in | std::ios::binary);
    response.directoryWriteHeader();
    while (fs.read(buf, READ_BUF_SIZE)){
        response.directWriteBody(buf, READ_BUF_SIZE);
    }
    left = fs.gcount();
    fs.read(buf, left);
    response.directWriteBody(buf, left);

    fs.close();
}

bool server::isPathMapped(std::string& method, std::string &path) {
    return mRouter[method].find(path) != mRouter[method].end();
}

void* server::handleClient(void* arg) {
    auto context = reinterpret_cast<handler_context*>(arg);
    int clnt_sock = context->clnt_sock;
    HttpRequest request(clnt_sock);
    auto* response = new DefaultHttpResponse(clnt_sock);
    context->mServer->handleRequest(request, *response);
    delete response;
    delete context;
    close(clnt_sock);
    return nullptr;
}