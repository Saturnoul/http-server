//
// Created by saturn on 4/15/21.
//

#include "body.h"
#include <sys/unistd.h>
#include "../../util/string_helper.h"

const char XWWW_DELIMETER = '&';
const char XWWW_KV_CONNECTOR = '=';
const int MAX_FORM_SIZE = 1024 * 1024 * 5;
const int INITIAL_FORM_SIZE = 1024;
const float FORM_FILE_EXPAND_FACTOR = 1.5f;

bool body_type::operator<(const body_type &other) const {
    return this->mName.compare(other.mName) < 0;
}

bool body_type::operator==(const body_type &other) const {
    return this->mName == other.mName;
}

body_type body_type::FORM = body_type("multipart/form-data");
body_type body_type::XWWW = body_type("application/x-www-form-urlencoded");
body_type body_type::JSON = body_type("application/json");
body_type body_type::EMTPY = body_type("empty");

int body_type::size() const{
    return mSize;
}


template<typename bodyType>
body* body::NewBody(sock_reader& sr, const body_type& type){
    if(type == body_type::EMTPY){
        return new body; 
    }

    auto b = new bodyType(type);
    b->parse(sr);
    return b;
}

std::map<const body_type, const std::function<body*(sock_reader&, const body_type&)>> body::BODY_TYPE_MAP = {
        {body_type::FORM, NewBody<FormData>},
        {body_type::XWWW, NewBody<XWWWFormUrlEncoded>},
        {body_type::JSON, NewBody<JsonData>},
        {body_type::EMTPY, [](sock_reader&, const body_type&)->body*{return new body;}}
};

body* body::createBody(const body_type& type, sock_reader& sr) {
    return BODY_TYPE_MAP[type](sr, type);
}

void body::parse(sock_reader& sr) {

}

void body::write(int clnt_sock) {
    ::write(clnt_sock, mData, mLen);
}

int body::size() const {
    return type.size();
}

void body::setData(const void *data, int len) {
    mData = reinterpret_cast<const char*>(data);
    mLen = len;
}


form_file::form_file(string &name) : mName(name){
    mData = new char[INITIAL_FORM_SIZE];
    mCapacity = INITIAL_FORM_SIZE;
    mCurLen = 0;
}


void form_file::write(char *buf, const int len) {
    if(mCurLen + len > mCapacity){
        int newCapacity = mCapacity * FORM_FILE_EXPAND_FACTOR;
        auto* newData = new char[newCapacity];
        memcpy(newData, mData, mCurLen);
        delete[] mData;
        mData = newData;
        mCapacity = newCapacity;
    }
    memcpy(mData + mCurLen, buf, len);
    mCurLen += len;
}

bool form_file::empty() {
    return mData == nullptr;
}

void form_file::setEmpty() {
    mData = nullptr;
}

const form_file& form_file::operator=(const form_file& other){
    if(this == &other){
        return *this;
    }
    mName = other.mName;
    mData = other.mData;
    mCurLen = other.mCurLen;
    mCapacity = other.mCapacity;
    return *this;
}

const form_file& form_file::operator=(form_file&& other) noexcept {
    this->mName = other.mName;
    this->mData = other.mData;
    this->mCapacity = other.mCapacity;
    this->mCurLen = other.mCurLen;
    other.setEmpty();
    return *this;
}

form_file::~form_file() {
    if(!mData){
        delete[] mData;
    }
}


void FormData::parse(sock_reader& sr) {
    auto boundary = getBoundary();
    auto start_boundary = boundary.insert(0, "--");
    auto end_boundary = boundary.append("--");
    int handledBodyLen = 0, bodySize = size();
    form_file* file = nullptr;
    sr.parseStream([this, &start_boundary, &end_boundary, &file, &handledBodyLen, bodySize](char* buf, int len, bool& nextRead, bool& nothingToRead) -> int {
        int handledLen = 0;
        if(file){
            int endBoundaryPos = BMSearch(buf, len, end_boundary.c_str(), end_boundary.length());
            int startBundaryPos = BMSearch(buf, len, start_boundary.c_str(), start_boundary.length());
            int retLen = 0;
            if(startBundaryPos < 0){
                file->write(buf, len);
                handledLen = len;
            } else{
                file->write(buf, startBundaryPos);
                file = nullptr;
                handledLen = endBoundaryPos < 0 ? startBundaryPos : len;
            }
        }
        else {
            int p = 0;
            string str(buf, len);
            while ((p = str.find(start_boundary, p)) != string::npos) {
                auto metaLine = next_line(str, p);
                if (metaLine.empty()) {
                    break;
                }
                if (isFile(metaLine)) {
                    auto name = getName(metaLine);
                    auto fileName = getFileName(metaLine);
                    if (next_n_line(str, p, 2).empty()) {
                        break;
                    }
                    mFiles[name] = form_file(fileName);
                    file = &mFiles[name];
                    handledLen = p + 3;
                    break;
                } else {
                    auto name = getName(metaLine);
                    string data = next_n_line(str, p, 2);
                    if (data.empty()) {
                        break;
                    }
                    mParams[name] = data;
                    handledLen = p + data.length() + 3;
                }
            }
        }
        nothingToRead = handledBodyLen + len == bodySize;
        handledBodyLen += handledLen;
        return handledLen;
    });
}

std::string FormData::getBoundary() {
    auto div = type.mData.find('=');
    return trim(type.mData.substr(div + 1));
}

bool FormData::isFile(string &line) {
    int part_num = 0;
    split(line, ';', [&part_num](const std::string&& str){
        part_num++;
    });
    return part_num == 3;
}

std::string FormData::getName(string &line) {
    auto p1 = line.find_first_of(';');
    auto p2 = line.find_first_of(';', p1 + 1);
    auto p3 = line.find('=', p1);
    return trim(line.substr(p3 + 1, p2 - p3 - 1), " \"");
}

std::string FormData::getFileName(string &line) {
    auto p = line.find_last_of('=');
    return trim(line.substr(p + 1), " \"");
}

const form_file& FormData::getFile(string &name) {
    return mFiles[name];
}

const std::string& FormData::getParam(string &name) {
    return mParams[name];
}


void XWWWFormUrlEncoded::parse(sock_reader& sr) {
    int handledBodyLen = 0, bodySize= size();
    sr.parseStream([this, &handledBodyLen, bodySize](char* buf, int len, bool& nextRead, bool& nothingToRead) -> int{
        nothingToRead = handledBodyLen + len == bodySize;
       int handled = split(string(buf, len), XWWW_DELIMETER, [this](const std::string&& str){
           auto pos = str.find(XWWW_KV_CONNECTOR);
           mParams[str.substr(0, pos)] = str.substr(pos + 1);
       }, !nothingToRead);
       handledBodyLen += handled;
       return handled;
    });
}

std::string &XWWWFormUrlEncoded::getParam(string &name) {
    return mParams[name];
}


Json::CharReaderBuilder JsonData::READER = Json::CharReaderBuilder();

void JsonData::parse(sock_reader& sr) {
    int handledBodyLen = 0, bodySize =size();
    std::string json_str;
    sr.parseStream([this, &handledBodyLen, bodySize, &json_str](char *buf, int len, bool &nextRead, bool &nothingToRead) -> int {
        nothingToRead = handledBodyLen + len == bodySize;
        json_str.append(buf, len);
        handledBodyLen += len;
        return len;
    });
    Json::IStringStream iss(json_str);
    std::string errs;
    Json::parseFromStream(READER, iss, &mJson, &errs);
}