//
// Created by saturn on 4/16/21.
//

#ifndef HTTP_IO_HELPER_H
#define HTTP_IO_HELPER_H

#include <functional>
#include <stdio.h>
#include <set>

#define SAFE_DELETE(p) \
delete p;              \
p = nullptr;


const int BUF_SIZE = 1024;

struct sock_reader_flag {
    bool nextRead;
    bool nothingToRead;
    bool skip;
};

typedef std::function<int(char *, int, sock_reader_flag& flag)> cb_type;

class sock_reader{
public:
    explicit sock_reader(int sock, bool keepListening = false);
    void parseStream(const cb_type& callback);
    void call_cb(const cb_type& callback, sock_reader_flag& flag);
    int getSocket()const;
    void peek(void* des, int n);
private:
    char buf[BUF_SIZE];
    char* tmp_buf;
    const int sock;
    int readLen, handledLen, maxReadLen, offset, curLen;
    bool keepListening;
};


class resource {
public:
    static resource* getInstance();
    static void init(const std::string& resource_root);

public:
    bool isStaticResource(std::string& path);
    std::string getFullPath(std::string path);
private:
    resource();
    explicit resource(const std::string& root);
    void buildResourceTable(const std::string& root);
private:
    std::string mResourceRoot;
    std::set<std::string> mResources;

    static resource* INSTANCE;
};

std::string joinPath(const std::string& prefix, const std::string& posix);

void makeRelativePath(std::string& path, int prefixLen);

#endif //HTTP_IO_HELPER_H
