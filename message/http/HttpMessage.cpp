//
// Created by saturn on 4/16/21.
//

#include <iostream>
#include "HttpMessage.h"


void HttpMessage::setHeader(const std::string& key, const std::string& value) {
    mHeader->setHeader(key, value);
}

const std::string& HttpMessage::getHeader(const std::string& key) const{
    return mHeader->getHeader(key);
}

body* HttpMessage::getBody() const{
    return mBody;
}

header* HttpMessage::getAllHeader() {
    return mHeader;
}

HttpMessage::HttpMessage() : mHeader(nullptr), mBody(nullptr) {

}

HttpMessage::~HttpMessage() {
    delete mHeader;
    delete mBody;
}
