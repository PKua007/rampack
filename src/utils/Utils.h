//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_UTILS_H
#define RAMPACK_UTILS_H

#include <iosfwd>
#include <vector>
#include <array>

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

bool endsWith(const std::string& str, const std::string& suffix);
bool startsWith(const std::string& str, const std::string& prefix);
int lastIndexOf(const std::string &s, char target);

void die(const std::string &reason);
void die(const std::string &reason, Logger &logger);

template <typename T>
std::size_t get_vector_memory_usage(const std::vector<T> &vec) {
    return vec.capacity() * sizeof(T);
}

#endif //RAMPACK_UTILS_H
