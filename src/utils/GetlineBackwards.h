//
// Created by Piotr Kubala on 13/06/2023.
//

#ifndef RAMPACK_GETLINEBACKWARDS_H
#define RAMPACK_GETLINEBACKWARDS_H

#include <istream>


/**
 * @brief Class for extracting lines from the stream in the reverse order.
 */
class GetlineBackwards {
public:
    /**
     * @brief Reads the line which @b precedes the current stream position.
     * @details <p>It reads all characters between [@a start, @a end) (@a start inclusive, @a end exclusive), where
     * @a end = `in.tellg()` and @a start is one character after first @a delim occurrence before @a end or the
     * beginning of the stream if reached before finding @a delim. The line is stored to @a line argument.
     *
     * <p> After the operation, `in.tellg()` points at the @a delim preceding the extracted line of the beginning or the
     * stream. Thus, subsequent method invocations on a stream with its read pointer at the end
     * `in.seekg(0, std::ios::end)` extract subsequent lines in the stream from the end.
     *
     * <p> It clears `std::ios::eofbit` flag from the stream state. If `in.tellg() == 0`, no preceding line can be
     * extracted and `std::ios::failbit` is set.
     * @param in the stream to extract the line from
     * @param line `std::string` to store the line
     * @param delim line delimiter
     * @return @a in
     */
    static std::istream &getline(std::istream &in, std::string &line, char delim = '\n');
};


#endif //RAMPACK_GETLINEBACKWARDS_H
