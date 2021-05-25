//
// Created by saturn on 4/26/21.
//

#include "WebsocketMessage.h"
#include <unistd.h>
#include "../../util/algorithm.h"

const char* REQUIRED_FIELD[] = {"Method", "Protocol", "Path", "Host", "Connection", "Upgrade", "Sec-WebSocket-Key", "Sec-WebSocket-Version"};
const char* REQUIRED_VALUE[] = {"GET", "HTTP/1.1", nullptr, nullptr, "Upgrade", "websocket", nullptr, "13"};
const int REQUIRED_LEN = 8;

const unsigned char C_80 = 0x80;
const unsigned char C_0F = 0x0F;
const unsigned char C_7F = 0x7F;

const int LEAST_BUF_LEN = 12;

const unsigned short PONG = 0x8A00;
const unsigned short CLOSE_FRAME = 0x8800;

WebsocketHandshake::WebsocketHandshake(sock_reader& sr) {
    mClntSock = sr.getSocket();
    mRequestHeader = new request_header(sr);
    mValid = verifyRequest(mRequestHeader);
    if(mValid){
        respond();
    }
}

WebsocketHandshake::WebsocketHandshake(int clnt_sock) {
    mClntSock = clnt_sock;
    mRequestHeader = new request_header;
}

int WebsocketHandshake::read(const char *buf, int len) {
    int readLen = mRequestHeader->read(buf, len);
    if(mRequestHeader->completed()) {
        mValid = verifyRequest(mRequestHeader);
        if(mValid) {
            respond();
        }
    }
    return readLen;
}

bool WebsocketHandshake::completed() const {
    return mRequestHeader->completed();
}

bool WebsocketHandshake::verifyRequest(const request_header* header) {
    const char* field, *value;
    for(int i = 0; i < REQUIRED_LEN; i++){
        field = REQUIRED_FIELD[i];
        value = REQUIRED_VALUE[i];
        if(!header->exist(field) || (value && header->getHeader(field) != value)){
            return false;
        }
    }
    return true;
}

void WebsocketHandshake::respond() {
    response_header responseHeader;
    responseHeader.setProtocol("HTTP/1.1");
    responseHeader.setStatusCode(101);

    auto clnt_key = mRequestHeader->getHeader("Sec-WebSocket-Key");
    auto srv_key = process_sec_websocket_key(clnt_key);

    responseHeader.setHeader("Upgrade", "websocket");
    responseHeader.setHeader("Connection", "Upgrade");
    responseHeader.setHeader("Sec-WebSocket-Accept", srv_key);

    responseHeader.write(mClntSock);
}

bool WebsocketHandshake::isValid() const {
    return mValid;
}

std::string WebsocketHandshake::getPath() {
    return mRequestHeader->getHeader("Path");
}

std::string WebsocketHandshake::getSecretKey() {
    return mRequestHeader->getHeader("Sec-WebSocket-Key");
}

int WebsocketHandshake::getSocket() const {
    return mClntSock;
}

void WebsocketHandshake::close() const {
    ::close(mClntSock);
}

WebsocketHandshake::WebsocketHandshake(HttpRequest &request, const int clnt_sock): mClntSock(clnt_sock) {
    mRequestHeader = dynamic_cast<request_header *>(request.mHeader);
    request.mHeader = nullptr;
    respond();
}

WebsocketHandshake::~WebsocketHandshake() {
    delete mRequestHeader;
}


WebsocketMessage::~WebsocketMessage() {
    delete[] data;
}

bool WebsocketMessage::completed() const {
    return mComplete;
}

int WebsocketMessage::read(const char* buf, int len) {
    if(mParsed) {
        write(buf, len);
        return len;
    }

    if(len < LEAST_BUF_LEN) {
        return 0;
    }
    unsigned char first_byte = buf[0], second_byte = buf[1];
    int cur_pos = 2;

    fin = static_cast<bool>(first_byte & C_80);
    op = static_cast<WebSocketOp>(first_byte & C_0F);
    mask = static_cast<bool>(second_byte & C_80);
    unsigned char payloadLen = second_byte & C_7F;
    if(payloadLen < 126){
        payload_len = payloadLen;
    }else if (payloadLen == 126) {
        short temp = 0;
        memcpy(&temp, buf + cur_pos, 2);
        payload_len = temp;
        cur_pos += 2;
    }else {
        memcpy(&payload_len, buf + cur_pos, 8);
        cur_pos += 8;
    }
    if(mask){
        memcpy(&mask_key, buf + cur_pos, 4);
        cur_pos += 4;
    }
    data = new char[payloadLen];
    write(buf + cur_pos, len - cur_pos);
    mParsed = true;
    return len;
}

void WebsocketMessage::decrypt() {
    if(!mask){
        return;
    }
    for(int i = 0; i < payload_len; i++){
        data[i] = data[i] ^ mask_key[i % 4];
    }
}

int WebsocketMessage::getSocket() const {
    return clnt_sock;
}

void WebsocketMessage::write(const char *buf, int len) {
    memcpy(data + mCurLen, buf, len);
    mCurLen += len;
    mComplete = mCurLen == payload_len;
}

void WebsocketMessage::reset() {
    mCurLen = 0;
    mComplete = false;
    mParsed = false;
    delete data;
}


WebsocketSession::WebsocketSession(WebsocketHandshake &handshake) {
    id = handshake.getSecretKey();
    clnt_sock = handshake.getSocket();
    path = handshake.getPath();
}

void WebsocketSession::sendPong() const {
    write(clnt_sock, &PONG, 2);
}

void WebsocketSession::sendClose() const {
    write(clnt_sock, &CLOSE_FRAME, 2);
    close(clnt_sock);
}

void WebsocketSession::sendMessage(const std::string& msg) const{
    unsigned short msg_len = msg.length();
    unsigned char first_byte = 0x81, second_byte = 0, payload_byte_len = 0;
    unsigned long long  frame_len = 0;
    if(msg_len < 126) {
        second_byte = static_cast<unsigned char>(msg_len);
    }else if (msg_len < 0xFFFF) {
        second_byte = 126;
        payload_byte_len = 2;
    }else {
        return;
    }
    frame_len = 2 + payload_byte_len + msg_len;

    auto* frame_data = new unsigned char[frame_len];
    frame_data[0] = first_byte;
    frame_data[1] = second_byte;
    memcpy(frame_data + 2, &msg_len, payload_byte_len);
    memcpy(frame_data + 2 + payload_byte_len, msg.c_str(), msg_len);
    write(clnt_sock, frame_data, frame_len);
    delete[] frame_data;
}

std::string WebsocketSession::getId() const {
    return id;
}

bool WebsocketSession::operator<(const WebsocketSession &other) const {
    return clnt_sock < other.clnt_sock;
}

std::string WebsocketSession::getPath() const {
    return path;
}
