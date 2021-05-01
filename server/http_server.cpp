//
// Created by saturn on 4/26/21.
//

#include "http_server.h"

#include <fstream>

const char *http_server_plugin::POST = "POST";
const char *http_server_plugin::GET = "GET";

const int READ_BUF_SIZE = 1024;

void http_server_plugin::post(const std::string &path, const handler_type &callback) {
    setHandler("POST", path, callback);
}

void http_server_plugin::get(const std::string &path, const handler_type &callback) {
    setHandler("GET", path, callback);
}

void http_server_plugin::handleHttpRequest(const HttpRequest &request, HttpResponse &response) {
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

void http_server_plugin::handleHttpRequest(const HttpRequest &request, int clnt_sock) {
    DefaultHttpResponse response(clnt_sock);
    handleHttpRequest(request, response);
}

void http_server_plugin::setHandler(const std::string &method, const std::string &path, const handler_type &handler) {
    mRouter[method][path] = handler;
}

handler_type &http_server_plugin::getHandler(const std::string &method, const std::string &path) {
    return mRouter[method][path];
}

void http_server_plugin::setStaticPath(const std::string &path) {
    resource::init(path);
}

bool http_server_plugin::isStaticResource(std::string &method, std::string &path) {
    if (method == GET && resource::getInstance()->isStaticResource(path)) {
        return true;
    }
    return false;
}

void http_server_plugin::handleStaticResource(std::string &path, HttpResponse &response) {
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

bool http_server_plugin::isPathMapped(std::string &method, std::string &path) {
    return mRouter[method].find(path) != mRouter[method].end();
}

void http_server::handle_connection(int clnt_sock, bool initial) {
    HttpRequest request(clnt_sock);
    DefaultHttpResponse response(clnt_sock);
    handleHttpRequest(request, response);
}
