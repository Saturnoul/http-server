//
// Created by saturn on 4/26/21.
//

#include "algorithm.h"
#include "../thirdparty/base64.h"
#include <openssl/ssl.h>
#include <cstring>

const char* WEBSOCKET_KEY_TAIL = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
const int WEBSOCKET_KEY_TAIL_LEN = 36;

std::string process_sec_websocket_key(std::string& key) {
    unsigned char sha_result[SHA_DIGEST_LENGTH];
    int key_len = key.length();
    int input_len = key_len + WEBSOCKET_KEY_TAIL_LEN;
    unsigned char input[input_len];

    memcpy(input, key.c_str(), key_len);
    memcpy(input + key_len, WEBSOCKET_KEY_TAIL, WEBSOCKET_KEY_TAIL_LEN);

    SHA1(input, input_len, sha_result);
    return base64_encode(sha_result, SHA_DIGEST_LENGTH);
}