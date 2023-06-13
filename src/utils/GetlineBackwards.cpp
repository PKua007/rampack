//
// Created by Piotr Kubala on 13/06/2023.
//

#include "GetlineBackwards.h"


std::istream &GetlineBackwards::getline(std::istream &in, std::string &line, char delim) {
    line.clear();

    if (in.fail())
        return in;

    if (in.tellg() == 0) {
        in.setstate(std::ios::failbit);
        return in;
    }

    if (in.eof())
        in.clear();

    bool keepSearching = true;
    std::streampos beforeLine;
    while (keepSearching) {
        in.seekg(-1, std::ios::cur);
        if (in.tellg() == 0 || in.peek() == delim) {
            keepSearching = false;
            beforeLine = in.tellg();
        }
    }

    if (in.peek() == delim)
        in.get();

    // Getline does not like getting EOF at the start and sets failbit - prevent it
    if (in.peek() != EOF)
        std::getline(in, line, delim);

    in.seekg(beforeLine);
    return in;
}
