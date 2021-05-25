//
// Created by saturn on 4/15/21.
//

#ifndef HTTP_BODY_H
#define HTTP_BODY_H

#include <string>
#include <map>
#include <json/json.h>
#include <utility>

#include "../../util/io_helper.h"

class body_type{
public:
    body_type()= default;
    body_type(std::string name) : mName(std::move(name)), mData(std::string()), mSize(0) {}
    body_type(std::string name, int size, std::string data=std::string()) : mName(std::move(name)), mData(std::move(data)), mSize(size) {};
public:
    bool operator < (const body_type& other) const;
    bool operator == (const body_type& other) const;
    int size()const;

    static body_type FORM;
    static body_type XWWW;
    static body_type JSON;
    static body_type EMTPY;
private:
    std::string mName;
    std::string mData;
    int mSize;

    friend class FormData;
};


class body {
public:
    body() : mData(""), mLen(0), mHandledBodyLen(0){};
    explicit body(body_type type): type(std::move(type)), mData(nullptr), mLen(0), mHandledBodyLen(0){};

public:
    template<typename bodyType>
    static body* NewBody(const body_type& type, sock_reader* sr);
    static body* createBody(const body_type& type, sock_reader* sr);

public:
    virtual void parse(sock_reader& sr);
    virtual void write(int clnt_sock);

    bool completed() const;
    virtual int read(const char* buf, int len);

public:
    int size() const;
    void setData(const void* data, int len);
private:
    static std::map<const body_type, const std::function<body*(const body_type&, sock_reader*)>> BODY_TYPE_MAP;
protected:
    body_type type;
    const char* mData;
    int mLen;
    bool mComplete;
    int mHandledBodyLen;
};


class form_file {
public:
    form_file() : mData(nullptr){}
    explicit form_file(std::string& name);

    ~form_file();

public:
    void write(const char *buf, const int len);
    bool empty();

private:
    void setEmpty();
    const form_file& operator=(const form_file& other);
    const form_file& operator=(form_file&& other) noexcept ;
private:
    std::string mName;
    char* mData;
    int mCurLen, mCapacity;

    friend class FormData;
};


class FormData : public body {
public:
    explicit FormData(const body_type& type);

public:
    void parse(sock_reader& sr) override;
    int read(const char* buf, int len) override;
    const form_file& getFile(std::string& name);
    const std::string& getParam(std::string& name);
private:
    std::string getBoundary();
    static bool isFile(std::string& line);
    static std::string getName(std::string& line);
    static std::string getFileName(std::string& line);

    std::string mStartBoundary;
    std::string mEndBoundary;
    form_file* file;

private:
    std::map<std::string, std::string> mParams;
    std::map<std::string, form_file> mFiles;
};


class XWWWFormUrlEncoded : public body {
public:
    explicit XWWWFormUrlEncoded(const body_type& type) : body(type) {}

public:
    void parse(sock_reader& sr) override;
    int read(const char* buf, int len) override;
    std::string& getParam(std::string& name);
private:
    std::map<std::string, std::string> mParams;
};


class JsonData : public body {
public:
    explicit JsonData(const body_type& type) : body(type) {}

public:
    void parse(sock_reader& sr) override;
    int read(const char *buf, int len) override;

    Json::Value mJson;
private:
    std::string json_str;
    static Json::CharReaderBuilder READER;
};

#endif //HTTP_BODY_H
