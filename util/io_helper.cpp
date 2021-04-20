//
// Created by saturn on 4/16/21.
//

#include "io_helper.h"
#include <unistd.h>
#include <cstring>


sock_reader::sock_reader(const int sock) : sock(sock), readLen(0), handledLen(0), offset(0), curLen(0), maxReadLen(BUF_SIZE) {
    memset(buf, 0, BUF_SIZE);
}

void sock_reader::parseStream(const  cb_type& callback) {
    bool nextRead = true;
    bool nothingToRead = false;

    tmp_buf = buf + offset;
    if (offset > 0) {
        readLen = 0;
        call_cb(callback, nextRead, nothingToRead);
    }
    while (!nothingToRead && nextRead && (readLen = read(sock, tmp_buf, maxReadLen)) > 0) {
        call_cb(callback, nextRead, nothingToRead);
    }
    if(nextRead && offset > 0){
        readLen = 0;
        call_cb(callback, nextRead, nothingToRead);
    }
}

void sock_reader::call_cb(const cb_type &callback, bool& nextRead, bool& nothingToRead) {
    curLen = readLen + offset;
    handledLen = callback(buf, curLen, nextRead, nothingToRead);
    offset = (curLen - handledLen) % BUF_SIZE;
    memcpy(buf, buf + handledLen, offset);
    maxReadLen = BUF_SIZE - offset;
    tmp_buf = buf + offset;
}
