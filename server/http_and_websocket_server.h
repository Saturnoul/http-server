//
// Created by saturn on 4/29/21.
//

#ifndef HTTP_HTTP_AND_WEBSOCKET_SERVER_H
#define HTTP_HTTP_AND_WEBSOCKET_SERVER_H

#include "server.h"
#include "websocket_server.h"
#include "http_server.h"
#include "../message/websocket/WebsocketMessage.h"

class http_and_websocket_server;

class http_and_websocket_connection : connection{
public:
    http_and_websocket_connection(int clnt_sock);
public:

    bool read(server* pServer) override;
private:
    bool isRequestTypeConfirmed;
    int mOffset;
    bool isMessage;
};

class http_and_websocket_server : public nonblocking_server<http_and_websocket_connection>, public http_server_plugin, public websocket_server_plugin{

};


#endif //HTTP_HTTP_AND_WEBSOCKET_SERVER_H
