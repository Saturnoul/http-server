//
// Created by saturn on 4/11/21.
//

#ifndef HTTP_HTTPREQUEST_H
#define HTTP_HTTPREQUEST_H

#include <cstring>
#include <string>
#include <unistd.h>

#include "HttpMessage.h"
#include "HttpSession.h"



class HttpRequest : public HttpMessage{
public:
    explicit HttpRequest(int sock);
    explicit HttpRequest(sock_reader& sr);
    HttpRequest(HttpRequest&& other) noexcept ;

public:
    void setHeader(const std::string& key, const std::string& value) override;
    const std::string& getPath() const;
    const std::string& getMethod() const;
    HttpSession& getSession() const;
    std::string getCookie() const;
private:
    void parse(int sock);
    void parse(sock_reader& sr);

private:
    static std::map<unsigned long long, HttpSession> SESSION_MAP;
    mutable std::string mCookie;

    friend class WebsocketHandshake;
};


#endif //HTTP_HTTPREQUEST_H
