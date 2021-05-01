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
    HttpRequest(HttpRequest&& other) noexcept ;

public:
    void setHeader(const std::string& key, const std::string& value) override;
    const std::string& getPath() const;
    const std::string& getMethod() const;
private:
    void parse(int sock);

    friend class WebsocketHandshake;
};


#endif //HTTP_HTTPREQUEST_H
