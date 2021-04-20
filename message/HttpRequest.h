//
// Created by saturn on 4/11/21.
//

#ifndef HTTP_HTTPREQUEST_H
#define HTTP_HTTPREQUEST_H

#include <cstring>
#include <string>
#include <unistd.h>

#include "HttpMessage.h"



class HttpRequest : public HttpMessage{
public:
    explicit HttpRequest(int sock);

public:
    void setHeader(const std::string& key, const std::string& value) override;
    const std::string& getPath();
    const std::string& getMethod();
private:
    void parse(int sock);
};


#endif //HTTP_HTTPREQUEST_H
