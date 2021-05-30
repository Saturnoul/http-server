//
// Created by saturn on 4/14/21.
//

#include "HttpResponse.h"
#include <unistd.h>


void HttpResponse::send() {
    checkHeader();
    write(clnt_sock, mRawData.data(), mRawData.length());
}

void HttpResponse::directlyWriteBody(char *data, int len) const {
    write(clnt_sock, data, len);
}

void HttpResponse::directlyWriteHeader() const {
    mHeader->write(clnt_sock);
}

void HttpResponse::setStatusCode(int code) {
    dynamic_cast<response_header*>(mHeader)->setStatusCode(code);
}

void HttpResponse::setProtocol(const std::string &protocol) {
    dynamic_cast<response_header*>(mHeader)->setProtocol(protocol);
}

void HttpResponse::setBody(void* data, int len) {
    mHeader->setHeader("Content-Length", std::to_string(len));
    mRawData = dynamic_cast<response_header*>(mHeader)->getRawData();
    mRawData.append(reinterpret_cast<char*>(data), len);
    mBodySet = true;
}

HttpResponse::HttpResponse(const int clnt_sock) : clnt_sock(clnt_sock){
    mHeader = new response_header;
    mBodySet = false;
}

void HttpResponse::end() const {
    close(clnt_sock);
}

void HttpResponse::checkHeader() {
    if(!mBodySet) {
        mRawData = dynamic_cast<response_header*>(mHeader)->getRawData();
    }
}

raw_data HttpResponse::getRawData() {
    checkHeader();
    return std::move(mRawData);
}


DefaultHttpResponse::DefaultHttpResponse(const int clnt_sock) : HttpResponse(clnt_sock){
    setStatusCode(200);
    setProtocol("HTTP/1.1");
    mHeader->setHeader("Server", "Saturn");
}
