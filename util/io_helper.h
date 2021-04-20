//
// Created by saturn on 4/16/21.
//

#ifndef HTTP_IO_HELPER_H
#define HTTP_IO_HELPER_H

#include <functional>

const int BUF_SIZE = 1024;

typedef std::function<int(char *, int, bool &, bool&)> cb_type;

class sock_reader{
public:
    explicit sock_reader(int sock);
    void parseStream(const cb_type& callback);
    void call_cb(const cb_type& callback, bool& nextRead, bool& nothingToRead);
private:
    char buf[BUF_SIZE];
    char* tmp_buf;
    const int sock;
    int readLen, handledLen, maxReadLen, offset, curLen;
};


void getAllFiles(std::string path, std::vector<std::string>& files);

#endif //HTTP_IO_HELPER_H
