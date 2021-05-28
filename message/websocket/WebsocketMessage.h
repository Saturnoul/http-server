//
// Created by saturn on 4/26/21.
//

#ifndef HTTP_WEBSOCKETMESSAGE_H
#define HTTP_WEBSOCKETMESSAGE_H

#include "../http/HttpRequest.h"
#include "../component/header.h"


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
    explicit WebsocketHandshake(int clnt_sock);
    explicit WebsocketHandshake(sock_reader& sr);
    explicit WebsocketHandshake(HttpRequest& request, int clnt_scok);
    ~WebsocketHandshake();
public:
    bool isValid() const;
    std::string getPath();
    std::string getSecretKey();
    int getSocket() const;
    void close() const;
    bool completed() const;

    int read(const char* buf, int len);

    static bool verifyRequest(const request_header* request);
private:
    void respond();
private:
    request_header* mRequestHeader;
    bool mValid = false;
    int mClntSock;

    friend class http_and_websocket_server;
    friend class websocket_server_plugin;
};


class WebsocketMessage{
public:
    WebsocketMessage() : clnt_sock(-1) {}
    explicit WebsocketMessage(int clnt_sock) : clnt_sock(clnt_sock) {}
    ~WebsocketMessage();

public:
    int read(const char* buf, int len);
    bool completed()const;
    int getSocket() const;
    void reset();

private:
    void decrypt();
    void write(const char* buf, int len);
private:
    bool fin = true, mask = true;
    WebSocketOp op = WebSocketOp::CLOSE;
    unsigned long long payload_len = 0;
    unsigned char mask_key[4]{};
private:
    char* data = nullptr;
    int mCurLen = 0;
    bool mComplete = false;
    bool mParsed = false;
    int clnt_sock;

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
