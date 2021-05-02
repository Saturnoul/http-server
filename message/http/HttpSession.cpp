//
// Created by saturn on 5/1/21.
//

#include "HttpSession.h"
#include <time.h>
#include <mutex>

std::mutex MUTEX;
int HttpSession::AUTO_INCRE_ID = 0;

void HttpSession::set(const std::string &k, const std::string &v) {
    mSession.insert(std::pair(k, v));
}

const std::string& HttpSession::get(const std::string &k) {
    return mSession.at(k);
}

unsigned long long HttpSession::generateId() {
    unsigned long long id = 0;
    auto time_stamp = time(nullptr);
    id |= time_stamp;
    id <<= 32;
    std::lock_guard lg(MUTEX);
    id |= AUTO_INCRE_ID;
    AUTO_INCRE_ID++;
    return id;
}
