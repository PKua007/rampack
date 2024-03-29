//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_UTILS_H
#define RAMPACK_UTILS_H

#include <iosfwd>
#include <vector>
#include <array>
#include <stdexcept>
#include <sstream>

#include "Logger.h"

// trim from start
std::string &ltrim(std::string &s);

// trim from end
std::string &rtrim(std::string &s);

// trim from both ends
std::string &trim(std::string &s);

// replaces all occurences of search in source by replace
std::string replaceAll(std::string source, const std::string& search, const std::string& replace);

std::vector<std::string> explode(const std::string &s, char delim);

template <typename Iter>
std::string implode(Iter begin, Iter end, const std::string &delim = ", ") {
    if (begin == end)
        return "";

    std::ostringstream out;
    for (auto it = begin; it != end; it++)
        out << *it << delim;

    std::string outStr = out.str();
    return outStr.substr(0, outStr.length() - delim.length());
}

template <typename Container>
std::string implode(const Container &container, const std::string &delim = ", ") {
    return implode(container.begin(), container.end(), delim);
}

bool isMultiline(const std::string &str);

bool endsWith(const std::string& str, const std::string& suffix);
bool startsWith(const std::string& str, const std::string& prefix);
int lastIndexOf(const std::string &s, char target);

std::string quoted(const std::string &str);

void die(const std::string &reason);
void die(const std::string &reason, Logger &logger);

template <typename T>
std::size_t get_vector_memory_usage(const std::vector<T> &vec) {
    return vec.capacity() * sizeof(T);
}

struct StringConversionException : public std::invalid_argument {
public:
    explicit StringConversionException(const std::string &what) : std::invalid_argument(what) { }
};

template <typename T>
T convert_string(const std::string &str) {
    std::istringstream in(str);
    T t;
    in >> t;
    if (!in)
        throw StringConversionException("Malformed string");

    in >> std::ws;
    std::string rest;
    std::getline(in, rest);
    if (!rest.empty())
        throw StringConversionException("Unexpected characters: " + rest);

    return t;
}

std::string demangle(const char *abiName);

template <class... T>
constexpr bool always_false = false;

#endif //RAMPACK_UTILS_H
