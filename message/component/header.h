//
// Created by saturn on 4/15/21.
//

#ifndef HTTP_HEADER_H
#define HTTP_HEADER_H

#include <string>
#include <map>

#include "../../util/io_helper.h"
#include "body.h"

class header {
public:
    void setHeader(const std::string& key, const std::string& value);
    const std::string& getHeader(const std::string& key);
    int getContentLength();
    virtual void write(int clnt_sock){}
protected:
    std::map<std::string, std::string> mHeaders;
    friend class HttpRequest;
};


class request_header : public header{
public:
    request_header() = default;
    explicit request_header(sock_reader& sr);
public:
    const char* getQuery(const std::string& key);
    body_type getContentType();
    void parse(sock_reader& sr);
private:
    void addHeader(const std::string&& header_line);
    void parseQuery(const std::string& path);
private:
    std::map<std::string, std::string> queries;
};


class response_header : public header {
public:
    void setStatusCode(int code);
    void setProtocol(const std::string& protocol);
private:
    void write(int serv_sock) override;

private:
    int mStatusCode;
    std::string mProtocol;

    static std::map<int, const char*> STATUS_MESSAGE;
};

#endif //HTTP_HEADER_H
