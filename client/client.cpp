//
// Created by saturn on 4/9/21.
//

#include <cstdio>
#include "client.h"

client client::ip(in_addr_t ip) {
    address.sin_addr.s_addr = ip;
    return *this;
}

client client::ip(const char* ip) {
    inet_aton(ip, &address.sin_addr);
    return *this;
}

client client::ip(long ip) {
    address.sin_addr.s_addr = htonl(ip);
    return *this;
}

client client::port(const char* port) {
    address.sin_port = htons(atoi(port));
    return *this;
}

client client::port(short port) {
    address.sin_port = port;
    return *this;
}

client client::build() {
    address.sin_family = AF_INET;
    clnt_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    return *this;
}

client::client() {
    memset(&address, 0, sizeof(address));
}

void client::start() {
    int ret = connect(clnt_socket, (const struct sockaddr*)&address, sizeof(address));
    char msg[] = "dsha";
    write(clnt_socket, msg, 4);
}