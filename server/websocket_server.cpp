//
// Created by saturn on 4/26/21.
//

#include "websocket_server.h"

void websocket_server::handle_connection(int clnt_sock, bool initial) {
    sock_reader sr(clnt_sock);
    if(initial){
        handleHandshake(clnt_sock);
    }else {
        handleFrame(clnt_sock);
    }
}

void websocket_server_plugin::addEndPoint(const std::string& path, const websocket_handler& handler) {
    mRouter[path] = handler;
}

void websocket_server_plugin::addSession(int clnt_sock, const WebsocketSession &session) {
    mSessionMap.insert(std::pair(clnt_sock, session));
}

void websocket_server_plugin::handleHandshake(WebsocketHandshake &handshake) {
    auto path = handshake.getPath();
    auto handler_pair = mRouter.find(path);
    if(handler_pair == mRouter.end()){
        handshake.close();
        return;
    }
    auto handler = handler_pair->second;
    WebsocketSession session(handshake);
    addSession(handshake.getSocket(), session);
    handshake.respond();
    handler.onOpen(session);
}

void websocket_server_plugin::handleHandshake(const int clnt_sock) {
    sock_reader sr(clnt_sock);
    WebsocketHandshake handshake(sr);
    if(handshake.isValid()){
        handleHandshake(handshake);
    }
}

bool websocket_server_plugin::handleFrame(const int clnt_sock) {
    sock_reader sr(clnt_sock);
    auto session_pair = mSessionMap.find(clnt_sock);
    if(session_pair == mSessionMap.end()){
        return false;
    }
    auto session = session_pair->second;
    auto path = session.getPath();
    auto handler = mRouter[path];
    bool continue_listen = true;

    sr.parseStream([&handler, &session, &continue_listen](char* buf, int len, bool& nextRead, bool& nothingToRead)->int{
        WebsocketMessage msg;
        msg.readFrame(buf, len);

        switch (msg.op) {
            case WebSocketOp::TEXT:
            case WebSocketOp::BINARY:
                msg.decrypt();
                handler.onMessage(session, std::string(msg.data, msg.payload_len));
                break;
            case WebSocketOp::PING:
                session.sendPong();
                break;
            case WebSocketOp::CLOSE:
                session.sendClose();
                handler.onClose(session);
                continue_listen = false;
                nextRead = false;
                break;
        }
        return len;
    });
    return continue_listen;
}
