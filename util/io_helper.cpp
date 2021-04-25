//
// Created by saturn on 4/16/21.
//

#include "io_helper.h"
#include "string_helper.h"
#include <unistd.h>
#include <cstring>
#include <dirent.h>
#include <algorithm>

#define IS_DIR(entry) (entry)->d_type == DT_DIR
#define IS_FILE(entry) (entry)->d_type == DT_REG
#define IS_CURRENT_DIR(entry) strcmp((entry)->d_name, ".") == 0
#define IS_PARENT_DIR(entry) strcmp((entry)->d_name, "..") == 0

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


resource* resource::INSTANCE = nullptr;

resource* resource::getInstance() {
    INSTANCE || (INSTANCE = new resource);
    return INSTANCE;
}

void resource::init(const std::string &root) {
    INSTANCE || (INSTANCE = new resource(root));
}

resource::resource() {

}

resource::resource(const std::string &root) {
    mResourceRoot = root;
    buildResourceTable(root);
}

void resource::buildResourceTable(const std::string &root) {
    struct dirent* entry = nullptr;
    DIR* dir = opendir(root.c_str());
    if(dir){
        while ((entry = readdir(dir))){
            if(IS_DIR(entry) && !IS_CURRENT_DIR(entry) && !IS_PARENT_DIR(entry)){
                buildResourceTable(joinPath(root, entry->d_name));
            } else if(IS_FILE(entry)){
                auto path = joinPath(root, entry->d_name);
                makeRelativePath(path, mResourceRoot.length());
                mResources.insert(path);
            }
        }
        closedir(dir);
    }
}

bool resource::isStaticResource(string &path) {
    return mResources.find(path) != mResources.end();
}

std::string resource::getFullPath(string path) {
#ifdef _WIN32
    replace_char(path, '/', '\\');
#endif
    return joinPath(mResourceRoot, path.substr(1));
}


std::string joinPath(const std::string& prefix, const std::string& posix) {
#ifdef _WIN32
    return std::move(prefix + "\\" + posix);
#else
    return prefix + "/" + posix;
#endif
}

void makeRelativePath(std::string& path, const int prefixLen) {
    path.erase(0, prefixLen);
#ifdef _WIN32
    replace_char(path, '\\', '/');
#endif
}