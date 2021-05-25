//
// Created by saturn on 4/26/21.
//

#include "websocket_server.h"

websocket_connection::websocket_connection(int clnt_sock) : connection(clnt_sock) {
    mOffset = 0;
    isHandshake = true;
    mRequest = new WebsocketHandshake(clnt_sock);
}

bool websocket_connection::read(server* pServer) {
    int readLen = 0;
    bool shouldKeepConnection = true;
    int len = ::recv(clnt_sock, mBuf + mOffset, SERVER_BUF_SIZE - mOffset, MSG_DONTWAIT);
    if(len <= 0){
        return true;
    }
    memcpy(mBuf, mBuf, mOffset);
    auto pWsServer = dynamic_cast<websocket_server*>(pServer);
    if(isHandshake) {
        auto handshake = reinterpret_cast<WebsocketHandshake*>(mRequest);
        readLen = handshake->read(mBuf, len + mOffset);
        if(handshake->completed()) {
            pWsServer->handleHandshake(*handshake);
            delete handshake;
            mRequest = new WebsocketMessage(clnt_sock);
            isHandshake = false;
        }
    } else {
        auto message = reinterpret_cast<WebsocketMessage*>(mRequest);
        readLen = message->read(mBuf, len + mOffset);
        if(message->completed()) {
            shouldKeepConnection = pWsServer->handleMessage(*message);
            message->reset();
        }
    }
    mOffset = len - readLen;
    memcpy(mBuf, mBuf + readLen, mOffset);

    return shouldKeepConnection;
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
    handler.onOpen(session);
}

void websocket_server_plugin::handleHandshake(const int clnt_sock) {
    sock_reader sr(clnt_sock);
    WebsocketHandshake handshake(sr);
    if(handshake.isValid()){
        handleHandshake(handshake);
    }
}

void websocket_server_plugin::handleFrame(const int clnt_sock) {
    sock_reader sr(clnt_sock);
    handleFrame(sr);
}

void websocket_server_plugin::handleFrame(sock_reader &sr) {
    sr.parseStream([this](char* buf, int len, sock_reader_flag& flag)->int{
        WebsocketMessage msg;
        int readLen = msg.read(buf, len);
        if(msg.completed()) {
            handleMessage(msg);
            flag.nothingToRead = true;
        }
        return readLen;
    });
}

bool websocket_server_plugin::handleMessage(WebsocketMessage &msg) {
    int clnt_sock = msg.clnt_sock;
    auto session_pair = mSessionMap.find(clnt_sock);
    if(session_pair == mSessionMap.end()){
        return false;
    }
    auto session = session_pair->second;
    auto path = session.getPath();
    auto handler = mRouter[path];

    bool complete = false;

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
            complete = true;
            handler.onClose(session);
            break;
    }
    return !complete;
}
