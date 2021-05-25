//
// Created by saturn on 4/26/21.
//

#include "http_server.h"

#include <stdio.h>

const char *http_server_plugin::POST = "POST";
const char *http_server_plugin::GET = "GET";

const int READ_mBuf_SIZE = 1024;

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
            auto cookie = request.getCookie();
            if(!cookie.empty()){
                response.setHeader("Set-Cookie", cookie);
            }
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
    char mBuf[READ_mBuf_SIZE];
    int readLen = 0;

    FILE* fp = fopen(resource::getInstance()->getFullPath(path).c_str(), "r");
    response.directoryWriteHeader();
    while ((readLen = fread(mBuf, 1, READ_mBuf_SIZE, fp)) > 0) {
        response.directWriteBody(mBuf, readLen);
    }
    response.end();
    fclose(fp);
}

bool http_server_plugin::isPathMapped(std::string &method, std::string &path) {
    return mRouter[method].find(path) != mRouter[method].end();
}


void http_server::handle_connection_thread(int clnt_sock) {
    HttpRequest request(clnt_sock);
    DefaultHttpResponse response(clnt_sock);
    handleHttpRequest(request, response);
}


http_connection::http_connection(int clnt_sock) : connection(clnt_sock) {
    mRequest = new HttpRequest;
}

bool http_connection::read(server* pServer) {
    int len = ::recv(clnt_sock, mBuf + mOffset, SERVER_BUF_SIZE - mOffset, MSG_DONTWAIT);
    if(len <= 0){
        return true;
    }
    memcpy(mBuf, mBuf, mOffset);
    int readLen = mRequest->read(mBuf, len);
    bool completed = mRequest->completed();
    if(completed) {
        auto pHttpServer = dynamic_cast<nonblocking_http_server*>(pServer);
        pHttpServer->handleHttpRequest(*mRequest, clnt_sock);
    } else {
        mOffset = len - readLen;
        memcpy(mBuf, mBuf + readLen, mOffset);
    }
    return !completed;
}

http_connection::~http_connection() {
    delete mRequest;
}
