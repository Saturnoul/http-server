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
    void setBody(void* data, int len);
    raw_data getRawData();
    void send();
    void end() const;

private:
    void directlyWriteBody(char* data, int len) const;
    void directlyWriteHeader() const;
    void checkHeader();
protected:
    void setProtocol(const std::string& protocol);

private:
    const int clnt_sock;
    raw_data mRawData;
    bool mBodySet;

    friend class http_server_plugin;
};


class DefaultHttpResponse : public HttpResponse {
public:
    explicit DefaultHttpResponse(int clnt_sock);
};


#endif //HTTP_HTTPRESPONSE_H
