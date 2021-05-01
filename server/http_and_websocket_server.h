//
// Created by saturn on 4/29/21.
//

#ifndef HTTP_HTTP_AND_WEBSOCKET_SERVER_H
#define HTTP_HTTP_AND_WEBSOCKET_SERVER_H

#include "server.h"
#include "websocket_server.h"
#include "http_server.h"
#include "../message/websocket/WebsocketMessage.h"


class http_and_websocket_server : public server, public http_server_plugin, public websocket_server_plugin{
public:
    void start_with_custom() override;
};


#endif //HTTP_HTTP_AND_WEBSOCKET_SERVER_H
