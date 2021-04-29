//
// Created by saturn on 4/14/21.
//

#include "HttpResponse.h"
#include <unistd.h>


void HttpResponse::send() {
    mHeader->write(clnt_sock);
    mBody->write(clnt_sock);
}

void HttpResponse::directWriteBody(char *data, int len) const {
    write(clnt_sock, data, len);
}

void HttpResponse::directoryWriteHeader() const {
    mHeader->write(clnt_sock);
}

void HttpResponse::setStatusCode(int code) {
    dynamic_cast<response_header*>(mHeader)->setStatusCode(code);
}

void HttpResponse::setProtocol(const std::string &protocol) {
    dynamic_cast<response_header*>(mHeader)->setProtocol(protocol);
}

void HttpResponse::setBody(body* b) {
    mBody = b;
    mHeader->setHeader("Content-Length", std::to_string(mBody->size()));
}

HttpResponse::HttpResponse(const int clnt_sock) : clnt_sock(clnt_sock){
    mHeader = new response_header;
    mBody = new body;
}

void HttpResponse::end() {
    close(clnt_sock);
}


DefaultHttpResponse::DefaultHttpResponse(const int clnt_sock) : HttpResponse(clnt_sock){
    setStatusCode(200);
    setProtocol("HTTP/1.1");
    mHeader->setHeader("Server", "Saturn");
}
