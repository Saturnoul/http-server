//
// Created by saturn on 4/11/21.
//

#include "HttpRequest.h"
#include "HttpResponse.h"

using namespace std;



HttpRequest::HttpRequest(const int sock){
    parse(sock);
}

void HttpRequest::parse(const int clnt_sock){
    sock_reader sr(clnt_sock);

    auto h = new request_header(sr);
    mBody = body::createBody(h->getContentType(), sr);
    mHeader = h;
}

void HttpRequest::setHeader(const std::string& key, const std::string& value) {
    //You can't modify the header of a http request
    return;
}

const std::string& HttpRequest::getPath() {
    return getHeader("Path");
}

const std::string &HttpRequest::getMethod() {
    return getHeader("Method");
}
