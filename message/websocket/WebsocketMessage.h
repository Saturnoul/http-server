//
// Created by saturn on 4/26/21.
//

#ifndef HTTP_WEBSOCKETMESSAGE_H
#define HTTP_WEBSOCKETMESSAGE_H

#include "../http/HttpRequest.h"

enum class WebSocketOp : unsigned char {
    CONTINUE = 0,
    TEXT = 1,
    BINARY = 2,
    CLOSE = 8,
    PING = 9,
    PONG = 10
};

class WebsocketHandshake{
public:
    explicit WebsocketHandshake(sock_reader& sr);
    explicit WebsocketHandshake(HttpRequest& request, int clnt_scok);
    ~WebsocketHandshake();
public:
    bool isValid() const;
    std::string getPath();
    std::string getSecretKey();
    int getSocket() const;
    void close() const;

    static bool verifyRequest(const request_header* request);
private:
    void respond();
private:
    request_header* mRequestHeader;
    bool mValid;
    int mClntSock;

    friend class http_and_websocket_server;
    friend class websocket_server_plugin;
};


class WebsocketMessage{
public:
    WebsocketMessage()=default;
    ~WebsocketMessage();

public:
    void readFrame(const char* frame, int len);
    bool isCompleted()const;

private:
    void decrypt();
private:
    bool fin = true, mask = true;
    WebSocketOp op = WebSocketOp::CLOSE;
    unsigned long long payload_len = 0;
    unsigned char mask_key[4]{};
    char* data = nullptr;
    bool completed = true;

    friend class websocket_server_plugin;
};


class WebsocketSession {
public:
    explicit WebsocketSession(WebsocketHandshake& handshake);

public:
    std::string getId() const;
    void sendMessage(const std::string& msg) const;
    void sendPong() const;
    void sendClose() const;
    std::string getPath() const;

    bool operator < (const WebsocketSession& other) const;
private:
    std::string id;
    std::string path;
    int clnt_sock;
};

#endif //HTTP_WEBSOCKETMESSAGE_H
