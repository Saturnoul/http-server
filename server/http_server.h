//
// Created by saturn on 4/26/21.
//

#ifndef HTTP_HTTP_SERVER_H
#define HTTP_HTTP_SERVER_H

#include <map>
#include <string>
#include "server.h"
#include "../message/http/HttpRequest.h"
#include "../message/http/HttpResponse.h"

typedef std::function<void(const HttpRequest&, HttpResponse&)> handler_type;

class http_server : public server{
public:
    void post(const std::string& path, const handler_type& callback);
    void get(const std::string& path, const handler_type& callback);
    void setStaticPath(const std::string& path);

private:
    void setHandler(const std::string& method, const std::string& path, const handler_type& handler);
    handler_type& getHandler(const std::string& method, const std::string& path);
    void handleHttpRequest(const HttpRequest& request, HttpResponse& response);
    bool isStaticResource(std::string& method, std::string& path);
    void handleStaticResource(std::string& path, HttpResponse& response);
    bool isPathMapped(std::string& method, std::string& path);

private:
    void handle_connection(int clnt_sock, bool initial) override;

private:
    static const char* POST;
    static const char* GET;
private:
    std::map<std::string, std::map<std::string, handler_type>> mRouter;
    friend class HttpMessage;
};


#endif //HTTP_HTTP_SERVER_H
