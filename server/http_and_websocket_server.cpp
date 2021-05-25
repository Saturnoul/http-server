//
// Created by saturn on 4/29/21.
//


#include <iostream>
#include "http_and_websocket_server.h"

const unsigned char C_40 = 0x40;

http_and_websocket_connection::http_and_websocket_connection(int clnt_sock) : connection(clnt_sock){
    isMessage = false;
    isRequestTypeConfirmed = false;
    mOffset = 0;
    mRequest = nullptr;
}

bool http_and_websocket_connection::read(server *pServer) {
    int readLen = 0;
    bool shouldKeepConnection = true;
    int len = ::recv(clnt_sock, mBuf + mOffset, SERVER_BUF_SIZE - mOffset, MSG_DONTWAIT);

    if(len <= 0){
        return true;
    }

    if(!isRequestTypeConfirmed) {
        // Since we don't support websocket extensions, the second bit of the first byte in a frame must be 0.
        isMessage = !static_cast<bool>(mBuf[0] & C_40);
        isRequestTypeConfirmed = true;
    }
    auto complex_server = dynamic_cast<http_and_websocket_server*>(pServer);
    if(isMessage){
        mRequest || (mRequest = new WebsocketMessage(clnt_sock));
        auto msg = reinterpret_cast<WebsocketMessage*>(mRequest);
        readLen = msg->read(mBuf, len);
        if(msg->completed()){
            shouldKeepConnection = complex_server->handleMessage(*msg);
            msg->reset();
        }
    }else{
        mRequest || (mRequest = new HttpRequest);
        auto request = reinterpret_cast<HttpRequest*>(mRequest);
        readLen = request->read(mBuf, len);
        if(request->completed()){
            auto req_header = dynamic_cast<const request_header*>(request->getAllHeader());
            if(WebsocketHandshake::verifyRequest(req_header)){
                WebsocketHandshake handshake(*request, clnt_sock);
                complex_server->handleHandshake(handshake);
                isMessage = true;
            }else {
                complex_server->handleHttpRequest(*request, clnt_sock);
                shouldKeepConnection = false;
//                    threadPool.enqueue([this, clnt_sock](const HttpRequest& request) {
//                        handleHttpRequest(request, clnt_sock);
//                    }, std::move(request));
            }
            SAFE_DELETE(mRequest)
        }
    }
    mOffset = len - readLen;
    memcpy(mBuf, mBuf + readLen, mOffset);
    return shouldKeepConnection;
}
