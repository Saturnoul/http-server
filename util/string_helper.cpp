//
// Created by saturn on 4/15/21.
//

#include "string_helper.h"

const int NO_OF_CHARS = 256;

int split(const string_view &s, const string_view &delimiters, function<void(const string_view &&)>&& callback, bool ignore_last) {
    string_view::size_type pos1 = 0, pos2 = s.find(delimiters);
    int len = delimiters.length();
    if (pos1 == pos2) {
        pos1 += len;
        pos2 = s.find(delimiters, pos1);
    }
    while (string_view::npos != pos2) {
        callback(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + len;
        pos2 = s.find(delimiters, pos1);
    }
    if (!ignore_last && pos1 != s.length()) {
        callback(s.substr(pos1));
    }
    return pos1;
}

int split(const string &s, const string &delimiters, function<void(const string &&)>&& callback, bool ignore_last) {
    string::size_type pos1 = 0, pos2 = s.find(delimiters);
    int len = delimiters.length();
    if (pos1 == pos2) {
        pos1 += len;
        pos2 = s.find(delimiters, pos1);
    }
    while (string::npos != pos2) {
        callback(s.substr(pos1, pos2 - pos1));

        pos1 = pos2 + len;
        pos2 = s.find(delimiters, pos1);
    }
    if (!ignore_last && pos1 != s.length()) {
        callback(s.substr(pos1));
    }
    return pos1;
}

int split(const string &s, const char delimiter, function<void(const string &&)>&& callback, bool ignore_last) {
    typedef function<void(const string&&)> func_type;
    return split(s, std::string(1, delimiter), std::forward<func_type&&>(callback), ignore_last);
}

void split(const string &&s, vector<std::string> &tokens, std::regex delimiters) {
    sregex_iterator begin = sregex_iterator(s.begin(), s.end(), delimiters);
    auto end = sregex_iterator();
    for (auto i = begin; i != end; i++) {
        smatch match = *i;
        string match_str = match.str();
        tokens.push_back(match_str);
    }
}

void strMoveLeft(string &s, int start, int begin, int end) {
    int len = s.length();
    if (start >= len ||
        begin >= len ||
        end > len ||
        start + end - begin >= len) {
        return;
    }
    for (int i = 0; i < end - begin; i++) {
        s.at(start + i) = s.at(i + begin);
    }
}

std::string next_line(const string& str, int& start) {
    std::string rn = "\r\n";
    int newLineStart = str.find(rn, start);
    if(newLineStart == string::npos){
        return string();
    }
    start = newLineStart + 1;
    int newLineEnd = str.find(rn, start);
    if (newLineEnd == string::npos){
        return string();
    }
    int len = newLineEnd - newLineStart - 2;
    if(len == 0){
        return string('\n', 1);
    }
    return str.substr(newLineStart + 2, len);
}

std::string next_n_line(const string& str, int& start, int n) {
    while (--n){
        if(next_line(str, start).empty()){
            return string();
        }
    }
    return next_line(str, start);
}

std::string trim(const string& str, const string& target){
    int left = 0, right = str.length() - 1;
    while (target.find(str[left]) != string::npos){
        left++;
    }
    while (target.find(str[right]) != string::npos){
        right--;
    }
    return str.substr(left, right - left + 1);
}

/* C++ Program for Bad Character Heuristic of Boyer
Moore String Matching Algorithm */

// The preprocessing function for Boyer Moore's
// bad character heuristic
void badCharHeuristic( const char* str, int size,
                       int badchar[NO_OF_CHARS])
{
    int i;

    // Initialize all occurrences as -1
    for (i = 0; i < NO_OF_CHARS; i++)
        badchar[i] = -1;

    // Fill the actual value of last occurrence
    // of a character
    for (i = 0; i < size; i++)
        badchar[(int) str[i]] = i;
}

/* A pattern searching function that uses Bad
Character Heuristic of Boyer Moore Algorithm */
int BMSearch( const char* txt, const int n, const char* pat, const int m)
{
//    int m = strlen(pat);
//    int n = strlen(txt);

    int badchar[NO_OF_CHARS];

    /* Fill the bad character array by calling
    the preprocessing function badCharHeuristic()
    for given pattern */
    badCharHeuristic(pat, m, badchar);

    int s = 0; // s is shift of the pattern with
    // respect to text
    while(s <= (n - m))
    {
        int j = m - 1;

        /* Keep reducing index j of pattern while
        characters of pattern and text are
        matching at this shift s */
        while(j >= 0 && pat[j] == txt[s + j])
            j--;

        /* If the pattern is present at current
        shift, then index j will become -1 after
        the above loop */
        if (j < 0)
        {
//            cout << "pattern occurs at shift = " << s << endl;

            /* Shift the pattern so that the next
            character in text aligns with the last
            occurrence of it in pattern.
            The condition s+m < n is necessary for
            the case when pattern occurs at the end
            of text */
//            s += (s + m < n)? m-badchar[txt[s + m]] : 1;
            return s;

        }

        else
            /* Shift the pattern so that the bad character
            in text aligns with the last occurrence of
            it in pattern. The max function is used to
            make sure that we get a positive shift.
            We may get a negative shift if the last
            occurrence of bad character in pattern
            is on the right side of the current
            character. */
            s += max(1, j - badchar[txt[s + j]]);
    }
    return -1;
}