//
// Created by saturn on 4/26/21.
//

#include "websocket_server.h"

void websocket_server::handle_connection(int clnt_sock, bool initial) {
    sock_reader sr(clnt_sock);
    if(initial){
        WebsocketHandshake handshake(sr);
        if(handshake.isValid()){
            auto path = handshake.getPath();
            auto handler_pair = mRouter.find(path);
            if(handler_pair == mRouter.end()){
                handshake.close();
                return;
            }
            auto handler = handler_pair->second;
            WebsocketSession session(handshake);
            addSession(clnt_sock, session);
            handler.onOpen(session);
        }
    }else {
        auto session_pair = mSessionMap.find(clnt_sock);
        if(session_pair == mSessionMap.end()){
            return;
        }
        auto session = session_pair->second;
        auto path = session.getPath();
        auto handler = mRouter[path];

        sr.parseStream([&handler, &session](char* buf, int len, bool& nextRead, bool& nothingToRead)->int{
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
                    nextRead = false;
                    break;
            }
            return len;
        });
    }
}

void websocket_server::addEndPoint(const std::string& path, const websocket_handler& handler) {
    mRouter[path] = handler;
}

void websocket_server::addSession(int clnt_sock, const WebsocketSession &session) {
    mSessionMap.insert(std::pair(clnt_sock, session));
}
