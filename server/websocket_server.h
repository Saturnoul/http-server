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

class websocket_server : public server{
public:
    void addEndPoint(const std::string& path, const websocket_handler& handler);
private:
    void handle_connection(int clnt_sock, bool initial) override;
    void addSession(int clnt_sock, const WebsocketSession& session);
private:
    std::map<std::string, websocket_handler> mRouter;
    std::map<int, WebsocketSession> mSessionMap;
};


#endif //HTTP_WEBSOCKET_SERVER_H
