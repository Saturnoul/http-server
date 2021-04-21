//
// Created by saturn on 4/15/21.
//

#ifndef HTTP_STRING_HELPER_H
#define HTTP_STRING_HELPER_H

#include <string>
#include <string_view>
#include <regex>
#include <functional>

using namespace std;

int split(const string_view& s, const string_view &delimiters, function<void(const string_view&&)>&& callback, bool ignore_last = false);

int split(const string& s, const string &delimiters, function<void(const string&&)>&& callback, bool ignore_last = false);

int split(const string& s, char delimiter, function<void(const string&&)>&& callback, bool ignore_last = false);

std::string trim(const string& str, const string& target = " ");

std::string next_line(const string& str, int& start);

std::string next_n_line(const string& str, int& start, int n);

int BMSearch( const char* txt, int m, const char* pat, int n);

void replace_char(std::string& str, char o, char n);

#endif //HTTP_STRING_HELPER_H
