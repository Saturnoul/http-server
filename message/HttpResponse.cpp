//
// Created by saturn on 4/14/21.
//

#include "HttpResponse.h"


void HttpResponse::write(int clnt_sock) {
    mHeader->write(clnt_sock);
    mBody->write(clnt_sock);
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

HttpResponse::HttpResponse() {
    mHeader = new response_header;
    mBody = new body();
}


DefaultHttpResponse::DefaultHttpResponse() {
    setStatusCode(200);
    setProtocol("HTTP/1.1");
    mHeader->setHeader("Server", "Saturn");

    mBody->setData("shanghai", 8);
}
