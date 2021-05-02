//
// Created by saturn on 4/11/21.
//

#include "HttpRequest.h"
#include "HttpResponse.h"
#include <regex>

using namespace std;

HttpRequest::HttpRequest(const int sock) {
    parse(sock);
}

HttpRequest::HttpRequest(sock_reader &sr) {
    parse(sr);
}

HttpRequest::HttpRequest(HttpRequest &&other) noexcept {
    this->mHeader = other.mHeader;
    this->mBody = other.mBody;
    other.mHeader = nullptr;
    other.mBody = nullptr;
}

void HttpRequest::parse(const int clnt_sock) {
    sock_reader sr(clnt_sock);
    parse(sr);
}

void HttpRequest::parse(sock_reader &sr) {
    auto h = new request_header(sr);
    mBody = body::createBody(h->getContentType(), sr);
    mHeader = h;
}

void HttpRequest::setHeader(const std::string &key, const std::string &value) {
    //You can't modify the header of a http request
    return;
}

const std::string &HttpRequest::getPath() const {
    return getHeader("Path");
}

const std::string &HttpRequest::getMethod() const {
    return getHeader("Method");
}

std::map<unsigned long long, HttpSession> HttpRequest::SESSION_MAP;

HttpSession &HttpRequest::getSession() const {
    if (mHeader->exist("Cookie")) {
        auto cookie_pair = getHeader("Cookie");
        std::regex session_pattern("(SESSIONID=)(.+)(;?)");
        std::smatch result;
        auto found = std::regex_search(cookie_pair, result, session_pattern);
        if (found) {
            auto session_str = result.str(2);
            auto sessionId = stoll(session_str);
            auto session_pair = SESSION_MAP.find(sessionId);
            if (session_pair != SESSION_MAP.end()) {
                return session_pair->second;
            }
        }
    }
    auto unique_id = HttpSession::generateId();
    mCookie.append("SESSIONID=").append(to_string(unique_id));
    return SESSION_MAP[unique_id];
}

std::string HttpRequest::getCookie() const {
    return mCookie;
}
