//
// Created by saturn on 4/16/21.
//

#include "io_helper.h"
#include <cstring>
#include <dirent.h>
#include <iostream>
#include <unistd.h>

#define IS_DIR(entry) (entry)->d_type == DT_DIR
#define IS_FILE(entry) (entry)->d_type == DT_REG
#define IS_CURRENT_DIR(entry) strcmp((entry)->d_name, ".") == 0
#define IS_PARENT_DIR(entry) strcmp((entry)->d_name, "..") == 0
const float EXPAND_FACTOR = 1.5f;

sock_reader::sock_reader(const int sock, bool keepListening) : sock(sock), readLen(0), handledLen(0), offset(0),
                                                               curLen(0), maxReadLen(BUF_SIZE),
                                                               keepListening(keepListening) {
    memset(buf, 0, BUF_SIZE);
}

void sock_reader::parseStream(const cb_type &callback) {
    sock_reader_flag flag{};
    flag.nextRead = true;
    flag.nothingToRead = false;
    int leastReadLen = keepListening ? -1 : 0;

    tmp_buf = buf;
    if (offset > 0) {
        readLen = 0;
        call_cb(callback, flag);
    }
    while (!flag.nothingToRead && flag.nextRead && (readLen = read(sock, tmp_buf, maxReadLen)) > leastReadLen) {
        call_cb(callback, flag);
    }

    if (flag.nextRead && offset > 0) {
        readLen = 0;
        call_cb(callback, flag);
    }
}

void sock_reader::call_cb(const cb_type &callback, sock_reader_flag& flag) {
    curLen = readLen + offset;
    handledLen = callback(buf, curLen, flag);
    offset = (curLen - handledLen) % BUF_SIZE;
    memcpy(buf, buf + handledLen, offset);
    maxReadLen = BUF_SIZE - offset;
    tmp_buf = buf + offset;
}

int sock_reader::getSocket() const {
    return sock;
}

void sock_reader::peek(void *des, int n) {
    if(offset == 0) {
        offset = read(sock, buf, maxReadLen);
    }
    memcpy(des, buf, n);
}


resource *resource::INSTANCE = nullptr;

resource *resource::getInstance() {
    INSTANCE || (INSTANCE = new resource);
    return INSTANCE;
}

void resource::init(const std::string &root) {
    INSTANCE || (INSTANCE = new resource(root));
}

resource::resource() = default;

resource::resource(const std::string &root) {
    mResourceRoot = root;
    buildResourceTable(root);
}

void resource::buildResourceTable(const std::string &root) {
    struct dirent *entry = nullptr;
    DIR *dir = opendir(root.c_str());
    if (dir) {
        while ((entry = readdir(dir))) {
            if (IS_DIR(entry) && !IS_CURRENT_DIR(entry) && !IS_PARENT_DIR(entry)) {
                buildResourceTable(joinPath(root, entry->d_name));
            } else if (IS_FILE(entry)) {
                auto path = joinPath(root, entry->d_name);
                makeRelativePath(path, mResourceRoot.length());
                mResources.insert(path);
            }
        }
        closedir(dir);
    }
}

bool resource::isStaticResource(const std::string &path) {
    return mResources.find(path) != mResources.end();
}

std::string resource::getFullPath(const std::string& path) {
#ifdef _WIN32
    replace_char(path, '/', '\\');
#endif
    return joinPath(mResourceRoot, path.substr(1));
}


std::string joinPath(const std::string &prefix, const std::string &posix) {
#ifdef _WIN32
    return std::move(prefix + "\\" + posix);
#else
    return prefix + "/" + posix;
#endif
}

void makeRelativePath(std::string &path, const int prefixLen) {
    path.erase(0, prefixLen);
#ifdef _WIN32
    replace_char(path, '\\', '/');
#endif
}


raw_data::raw_data(int capacity) : mCapacity(capacity) {
    mData = new char[mCapacity];
    mLen = 0;
}

raw_data& raw_data::append(const std::string &str) {
    return append(str.data(), str.length());
}

raw_data& raw_data::append(const char *data) {
    return append(data, strlen(data));
}

raw_data& raw_data::append(const int data) {
    return append(std::to_string(data));
}

raw_data& raw_data::append(const char *data, int len) {
    if(mLen + len > mCapacity) {
        mCapacity = static_cast<int>(EXPAND_FACTOR * mCapacity);
        auto newData = new char[mCapacity];
        memcpy(newData, mData, mLen);
        delete mData;
        mData = newData;
    }
    memcpy(mData + mLen, data, len);
    mLen += len;
    return *this;
}

raw_data& raw_data::operator=(raw_data &&other) noexcept{
    if(this == &other) {
        return *this;
    }
    mLen = other.mLen;
    mCapacity = other.mCapacity;
    mData = other.mData;
    other.mData = nullptr;
    return *this;
}

raw_data::raw_data(raw_data &&other) noexcept{
    mLen = other.mLen;
    mCapacity = other.mCapacity;
    mData = other.mData;
    other.mData = nullptr;
}

raw_data::~raw_data() {
    delete mData;
}
