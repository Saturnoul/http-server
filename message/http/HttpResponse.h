//
// Created by saturn on 4/14/21.
//

#ifndef HTTP_HTTPRESPONSE_H
#define HTTP_HTTPRESPONSE_H

#include "HttpMessage.h"

class HttpResponse : public HttpMessage{
public:
    explicit HttpResponse(int clnt_sock);
public:
    void setStatusCode(int code);
    void setBody(body* b);
    void send();
    void end();

private:
    void directWriteBody(char* data, int len) const;
    void directoryWriteHeader() const;
protected:
    void setProtocol(const std::string& protocol);

private:
    const int clnt_sock;

    friend class http_server_plugin;
};


class DefaultHttpResponse : public HttpResponse {
public:
    explicit DefaultHttpResponse(int clnt_sock);
};


#endif //HTTP_HTTPRESPONSE_H
