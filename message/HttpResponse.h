//
// Created by saturn on 4/14/21.
//

#ifndef HTTP_HTTPRESPONSE_H
#define HTTP_HTTPRESPONSE_H

#include "HttpMessage.h"

class HttpResponse : public HttpMessage{
public:
    HttpResponse();
public:
    void setStatusCode(int code);
    void setBody(body* b);
    void write(int clnt_sock);

protected:
    void setProtocol(const std::string& protocol);
};


class DefaultHttpResponse : public HttpResponse {
public:
    DefaultHttpResponse();
};


#endif //HTTP_HTTPRESPONSE_H
