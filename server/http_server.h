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

class http_server_plugin {
public:
    void post(const std::string& path, const handler_type& callback);
    void get(const std::string& path, const handler_type& callback);
    static void setStaticPath(const std::string& path);

public:
    void handleHttpRequest(const HttpRequest& request, HttpResponse& response);
    void handleHttpRequest(const HttpRequest& request, int clnt_sock);
    static bool isStaticResource(std::string& method, std::string& path);
    static void handleStaticResource(std::string& path, HttpResponse& response);
    bool isPathMapped(std::string& method, std::string& path);

protected:
    void setHandler(const std::string& method, const std::string& path, const handler_type& handler);
    handler_type& getHandler(const std::string& method, const std::string& path);

private:
    static const char* POST;
    static const char* GET;
private:
    std::map<std::string, std::map<std::string, handler_type>> mRouter;
};


class http_connection : public connection{
public:
    http_connection(int clnt_sock);
    ~http_connection() override;
public:
    bool read(server* pServer) override;

private:
    int mOffset = 0;
    HttpRequest* mRequest;
};


class http_server : public server, public http_server_plugin{
private:
    void handle_connection_thread(int clnt_sock) override;
};


class nonblocking_http_server : public nonblocking_server<http_connection>, public http_server_plugin {
};

#endif //HTTP_HTTP_SERVER_H
