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

const unsigned short PONG = 0x8A00;
const unsigned short CLOSE_FRAME = 0x8800;

WebsocketHandshake::WebsocketHandshake(sock_reader& sr) {
    mClntSock = sr.getSocket();
    mRequestHeader = request_header(sr);
    mValid = verifyRequest();
    if(mValid){
        respond();
    }
}

bool WebsocketHandshake::verifyRequest() {
    const char* field, *value;
    for(int i = 0; i < REQUIRED_LEN; i++){
        field = REQUIRED_FIELD[i];
        value = REQUIRED_VALUE[i];
        if(!mRequestHeader.exist(field) || (value && mRequestHeader.getHeader(field) != value)){
            return false;
        }
    }
    return true;
}

void WebsocketHandshake::respond() {
    response_header responseHeader;
    responseHeader.setProtocol("HTTP/1.1");
    responseHeader.setStatusCode(101);

    auto clnt_key = mRequestHeader.getHeader("Sec-WebSocket-Key");
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
    return mRequestHeader.getHeader("Path");
}

std::string WebsocketHandshake::getSecretKey() {
    return mRequestHeader.getHeader("Sec-WebSocket-Key");
}

int WebsocketHandshake::getSocket() const {
    return mClntSock;
}

void WebsocketHandshake::close() {
    ::close(mClntSock);
}


WebsocketMessage::~WebsocketMessage() {
    delete[] data;
}

bool WebsocketMessage::isCompleted() const {
    return completed;
}

void WebsocketMessage::readFrame(const char *frame, int len) {
    unsigned char first_byte = frame[0], second_byte = frame[1];
    int cur_pos = 2;

    fin = static_cast<bool>(first_byte & C_80);
    op = static_cast<WebSocketOp>(first_byte & C_0F);
    mask = static_cast<bool>(second_byte & C_80);
    unsigned char payloadLen = second_byte & C_7F;
    if(payloadLen < 126){
        payload_len = payloadLen;
    }else if (payloadLen == 126) {
        short temp = 0;
        memcpy(&temp, frame + cur_pos, 2);
        payload_len = temp;
        cur_pos += 2;
    }else {
        memcpy(&payload_len, frame + cur_pos, 8);
        cur_pos += 8;
    }
    if(mask){
        memcpy(&mask_key, frame + cur_pos, 4);
        cur_pos += 4;
    }
    data = new char[payloadLen];
    memcpy(data, frame + cur_pos, payloadLen);
}

void WebsocketMessage::decrypt() {
    if(!mask){
        return;
    }
    for(int i = 0; i < payload_len; i++){
        data[i] = data[i] ^ mask_key[i % 4];
    }
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
