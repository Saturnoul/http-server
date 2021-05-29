//
// Created by saturn on 4/26/21.
//

#ifndef HTTP_WEBSOCKET_SERVER_H
#define HTTP_WEBSOCKET_SERVER_H

#include "server.h"
#include "../message/websocket/WebsocketMessage.h"
#include <map>

struct websocket_handler {
    std::function<void(const WebsocketSession& session)> onOpen;
    std::function<void(const WebsocketSession& session)> onClose;
    std::function<void(const WebsocketSession& session, const std::string& msg)> onMessage;
    std::function<void(const WebsocketSession& session, const std::string& err)> onError;
};

class websocket_server_plugin{
public:
    void addEndPoint(const std::string& path, const websocket_handler& handler);
    void handleHandshake(int clnt_sock);
    void handleHandshake(WebsocketHandshake& handshake);
    bool handleMessage(WebsocketMessage& message);
    void handleFrame(int clnt_sock);
    void handleFrame(sock_reader& sr);
private:
    void addSession(int clnt_sock, const WebsocketSession& session);
private:
    std::map<std::string, websocket_handler> mRouter;
    std::map<int, WebsocketSession> mSessionMap;
};


class websocket_connection : public connection{
public:
    websocket_connection(int clnt_sock, int epfd);
public:
    bool read(server* pServer) override;
private:
    int mOffset;
    void* mRequest;
    bool isHandshake;
};


class websocket_server : public nonblocking_server<websocket_connection>, public websocket_server_plugin{
};

#endif //HTTP_WEBSOCKET_SERVER_H
