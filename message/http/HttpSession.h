//
// Created by saturn on 5/1/21.
//

#ifndef HTTP_HTTPSESSION_H
#define HTTP_HTTPSESSION_H

#include <string>
#include <map>

class HttpSession {
public:
    void set(const std::string& k, const std::string& v);
    const std::string& get(const std::string& k);

public:
    static unsigned long long generateId();

public:
    std::map<std::string, std::string> mSession;

    static int AUTO_INCRE_ID;
};


#endif //HTTP_HTTPSESSION_H
