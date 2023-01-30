//
// Created by Piotr Kubala on 12/12/2020.
//

#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <utility>
#include <iomanip>

#include <cxxabi.h>

#include "Utils.h"
#include "Exceptions.h"


// trim from start
std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) { return !std::isspace(c); }));
    return s;
}

// trim from end
std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) { return !std::isspace(c); }).base(), s.end());
    return s;
}

// trim from both ends
std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

bool endsWith(const std::string &str, const std::string &suffix) {
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

bool startsWith(const std::string &str, const std::string &prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

std::string replaceAll(std::string source, const std::string& search, const std::string& replace) {
    size_t pos = 0;
    while ((pos = source.find(search, pos)) != std::string::npos) {
        source.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return source;
}

int lastIndexOf(const std::string &s, char target){
    int ret = -1;
    int curIdx = 0;
    while(s[curIdx] != '\0'){
        if (s[curIdx] == target) ret = curIdx;
        curIdx++;
    }
    return ret;
}

std::vector<std::string> explode(const std::string &s, char delim) {
    if (s.empty())
        return {};

    std::vector<std::string> result;
    std::istringstream iss(s);

    std::string token;
    while(std::getline(iss, token, delim))
        result.push_back(std::move(token));

    if (s.back() == delim)
        result.push_back("");

    return result;
}

void die(const std::string &reason) {
    std::cerr << reason << std::endl;
    exit(EXIT_FAILURE);
}

void die(const std::string &reason, Logger &logger) {
    logger.error() << reason << std::endl;
    exit(EXIT_FAILURE);
}

std::string demangle(const char *abiName) {
    char *demangledCstr = abi::__cxa_demangle(abiName, nullptr, nullptr, nullptr);
    if (demangledCstr == nullptr)
        return abiName;

    std::string demangledStr(demangledCstr);
    std::free(demangledCstr);
    return demangledStr;
}

std::string quoted(const std::string &str) {
    std::ostringstream out;
    out << std::quoted(str);
    return out.str();
}

bool isMultiline(const std::string &str) {
    return str.find('\n') != std::string::npos;
}

