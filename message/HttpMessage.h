//
// Created by saturn on 4/16/21.
//

#ifndef HTTP_HTTPMESSAGE_H
#define HTTP_HTTPMESSAGE_H


#include "component/header.h"
#include "component/body.h"


class HttpMessage {
public:
    HttpMessage();
    virtual ~HttpMessage();
public:
    virtual void setHeader(const std::string& key, const std::string& value);
    const std::string& getHeader(const std::string& key) const;
    header* getAllHeader();
    body* getBody() const;

protected:
    header* mHeader;
    body* mBody;
};


#endif //HTTP_HTTPMESSAGE_H
