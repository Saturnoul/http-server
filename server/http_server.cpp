//
// Created by saturn on 4/26/21.
//

#include "http_server.h"

#include <stdio.h>

const char *http_server_plugin::POST = "POST";
const char *http_server_plugin::GET = "GET";

const int READ_BUF_SIZE = 1024;

void http_server_plugin::post(const std::string &path, const handler_type &callback) {
    setHandler("POST", path, callback);
}

void http_server_plugin::get(const std::string &path, const handler_type &callback) {
    setHandler("GET", path, callback);
}

void http_server_plugin::handleRequest(const HttpRequest &request, HttpResponse &response) {
    const auto& method = request.getMethod();
    const auto& path = request.getPath();
    if (isStaticResource(method, path)) {
        handleStaticResource(path, response);
    } else {
        handleHttpRequest(request, response);
        response.send();
    }
    response.end();
}

void http_server_plugin::handleRequest(int clnt_sock) {
    HttpRequest request(clnt_sock);
    DefaultHttpResponse response(clnt_sock);
    handleRequest(request, response);
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

bool http_server_plugin::isStaticResource(const std::string &method, const std::string &path) {
    if (method == GET && resource::getInstance()->isStaticResource(path)) {
        return true;
    }
    return false;
}

bool http_server_plugin::isStaticResource(const HttpRequest &request) {
    const auto& method = request.getMethod();
    const auto& path = request.getPath();
    return isStaticResource(method, path);
}

void http_server_plugin::handleStaticResource(const std::string &path, HttpResponse &response) {
    char mBuf[READ_BUF_SIZE];
    int readLen = 0;

    FILE* fp = fopen(resource::getInstance()->getFullPath(path).c_str(), "r");
    response.directlyWriteHeader();
    while ((readLen = fread(mBuf, 1, READ_BUF_SIZE, fp)) > 0) {
        response.directlyWriteBody(mBuf, readLen);
    }
    fclose(fp);
}

void http_server_plugin::handleHttpRequest(const HttpRequest &request, HttpResponse &response) {
    const auto& method = request.getMethod();
    const auto& path = request.getPath();
    if (isPathMapped(method, path)) {
        getHandler(method, path)(request, response);
        auto cookie = request.getCookie();
        if(!cookie.empty()){
            response.setHeader("Set-Cookie", cookie);
        }
    } else {
        response.setStatusCode(404);
    }
}

bool http_server_plugin::isPathMapped(const std::string &method, const std::string &path) {
    return mRouter[method].find(path) != mRouter[method].end();
}


void http_server::handle_connection_thread(int clnt_sock) {
    handleRequest(clnt_sock);
}


ThreadPool* http_connection::THREAD_POOL = new ThreadPool(THREAD_POOL_SIZE);

http_connection::http_connection(int clnt_sock) : connection(clnt_sock) {
    mRequest = new HttpRequest;
}

bool http_connection::read(server* pServer) {
    int len = ::recv(clnt_sock, mBuf + mOffset, SERVER_BUF_SIZE - mOffset, MSG_DONTWAIT);
    if(len <= 0){
        return false;
    }
    auto request = reinterpret_cast<HttpRequest*>(mRequest);
    int readLen = request->read(mBuf, len);
    bool completed = request->completed();
    if(completed) {
        auto pHttpServer = dynamic_cast<nonblocking_http_server*>(pServer);
        THREAD_POOL->enqueue([this, pHttpServer](const HttpRequest& request){
            DefaultHttpResponse response(clnt_sock);
            if(pHttpServer->isStaticResource(request)) {
                pHttpServer->handleStaticResource(request.getPath(), response);
                response.end();
            }else {
                pHttpServer->handleHttpRequest(request, response);
                auto rawData = response.getRawData();
                setData(rawData.data(), rawData.length());
                rawData.clear();
            }
        }, std::move(*request));
    } else {
        mOffset = len - readLen;
        memcpy(mBuf, mBuf + readLen, mOffset);
    }
    return completed;
}