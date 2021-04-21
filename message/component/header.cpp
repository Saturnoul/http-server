//
// Created by saturn on 4/15/21.
//

#include "header.h"
#include <sys/unistd.h>
#include "../../util/string_helper.h"

const char EQUAL = '=';
const char AND = '&';
const char QUESTION = '?';
const char SPACE = ' ';
const char SEMICOLON = ':';
const char* CRLF = "\r\n";

const std::string HEADER_BODY_SEPARATOR = "\r\n\r\n";

void header::setHeader(const std::string &key, const std::string &value) {
    mHeaders[key] = trim(value);
}

const std::string &header::getHeader(const std::string &key) {
    return mHeaders[key];
}

int header::getContentLength() {
    if(mHeaders.find("Content-Length") == mHeaders.end()){
        return -1;
    }
    return stoi(mHeaders["Content-Length"]);
}


request_header::request_header(sock_reader &sr) {
    parse(sr);
}

void request_header::addHeader(const std::string &&header) {
    auto pos = header.find(SEMICOLON);
    if (pos != std::string::npos) {
        setHeader(header.substr(0, pos), header.substr(pos + 1));
    } else {
        auto p1 = 0;
        auto p2 = header.find_first_of(SPACE);
        auto p3 = header.find_last_of(SPACE);
        setHeader("Method", header.substr(p1, p2 - p1));
        setHeader("Path", header.substr(p2 + 1, p3 - p2 - 1));
        setHeader("Protocol", header.substr(p3 + 1));

        parseQuery(getHeader("Path"));
    }
}

const char *request_header::getQuery(const std::string &key) {
    return queries[key].c_str();
}

void request_header::parseQuery(const std::string &path) {
    auto pos = path.find(QUESTION);
    if (pos != std::string::npos) {
        split(path.substr(pos + 1), AND, [this](const std::string &&str) {
            auto p = str.find(EQUAL);
            this->queries[str.substr(0, p)] = str.substr(p + 1);
        });
    }
}

void request_header::parse(sock_reader &sr) {
    string msg;
    sr.parseStream([this, &msg](char *buf, int len, bool &nextRead, bool nothingToRead) -> int {
        msg.append(buf, len);
        auto headerEnd = msg.find(HEADER_BODY_SEPARATOR);
        if (headerEnd != string::npos) {
            auto cb = std::bind(&request_header::addHeader, this, placeholders::_1);
            split(msg.substr(0, headerEnd), "\r\n", cb);
            nextRead = false;
            return headerEnd + 4;
        }
        return len;
    });
}

body_type request_header::getContentType() {
    std::string ct = getHeader("Content-Type");
    int contentLen = getContentLength();
    if(contentLen < 0){
        return body_type::EMTPY;
    }
    auto div = ct.find(';');
    if (string::npos != div) {
        return body_type(ct.substr(0, div), contentLen, ct.substr(div + 1));
    } else {
        return body_type(ct, contentLen);
    }
}


void response_header::setStatusCode(int code) {
    mStatusCode = code;
}

void response_header::setProtocol(const std::string& protocol) {
    mProtocol = protocol;
}

void response_header::write(int clnt_sock) {
    std::string header_str;
    header_str.append(mProtocol).append(" ").append(to_string(mStatusCode)).append(" ").append(STATUS_MESSAGE[mStatusCode]);
    std::map<std::string, std::string>::iterator iter;
    for(iter = mHeaders.begin(); iter != mHeaders.end(); iter++){
        header_str.append(CRLF).append(iter->first).append(": ").append(iter->second);
    }
    header_str.append(CRLF).append(CRLF);
    ::write(clnt_sock, header_str.data(), header_str.length());
}


std::map<int, const char *> response_header::STATUS_MESSAGE = {
        {100, "Continue"},
        {101, "Switching Protocols"},
        {200, "OK"},
        {201, "Created"},
        {202, "Accepted"},
        {203, "Non-Authoritative Information"},
        {204, "No Content"},
        {205, "Reset Content"},
        {206, "Partial Content"},
        {300, "Multiple Choices"},
        {301, "Moved Permanently"},
        {302, "Found"},
        {303, "See Other"},
        {304, "Not Modified"},
        {305, "Use Proxy"},
        {307, "Temporary Redirect"},
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {402, "Payment Required"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {405, "Method Not Allowed"},
        {406, "Not Acceptable"},
        {407, "Proxy Authentication Required"},
        {408, "Request Time-out"},
        {409, "Conflict"},
        {410, "Gone"},
        {411, "Length Required"},
        {412, "Precondition Failed"},
        {413, "Request Entity Too Large"},
        {414, "Request-URI Too Large"},
        {415, "Unsupported Media Type"},
        {416, "Requested range not satisfiable"},
        {417, "Expectation Failed"},
        {500, "Internal Server Error"},
        {501, "Not Implemented"},
        {502, "Bad Gateway"},
        {503, "Service Unavailable"},
        {504, "Gateway Time-out"},
        {505, "HTTP Version not supported"}
};
