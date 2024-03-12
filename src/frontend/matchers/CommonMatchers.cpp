//
// Created by Piotr Kubala on 12/03/2024.
//

#include "CommonMatchers.h"


using namespace pyon::matcher;


MatcherString CommonMatchers::symbol = MatcherString{}
    .nonEmpty()
    .filter([](const std::string &str){
        if (!std::all_of(str.begin(), str.end(), [](char c) { return c == '_' || std::isalnum(c); }))
            return false;
        if (std::isdigit(str.front()))
            return false;
        return true;
    })
    .describe("valid symbol name (only letters, numbers and underscore; doesn't start with a number)");