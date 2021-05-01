//
// Created by saturn on 4/11/21.
//

#include "HttpRequest.h"
#include "HttpResponse.h"

using namespace std;



HttpRequest::HttpRequest(const int sock){
    parse(sock);
}

HttpRequest::HttpRequest(sock_reader& sr) {
    parse(sr);
}

HttpRequest::HttpRequest(HttpRequest &&other)  noexcept {
    this->mHeader = other.mHeader;
    this->mBody = other.mBody;
    other.mHeader = nullptr;
    other.mBody = nullptr;
}

void HttpRequest::parse(const int clnt_sock){
    sock_reader sr(clnt_sock);
    parse(sr);
}

void HttpRequest::parse(sock_reader& sr) {
    auto h = new request_header(sr);
    mBody = body::createBody(h->getContentType(), sr);
    mHeader = h;
}

void HttpRequest::setHeader(const std::string& key, const std::string& value) {
    //You can't modify the header of a http request
    return;
}

const std::string& HttpRequest::getPath() const{
    return getHeader("Path");
}

const std::string &HttpRequest::getMethod() const{
    return getHeader("Method");
}
