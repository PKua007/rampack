//
// Created by Piotr Kubala on 13/06/2023.
//

#include <catch2/catch.hpp>
#include <sstream>

#include "utils/GetlineBackwards.h"


TEST_CASE("GetlineBackwards: general") {
    std::istringstream in("first line\n\nthird line\n");
    in.seekg(0, std::ios::end);

    std::string line;

    REQUIRE(GetlineBackwards::getline(in, line));
    CHECK(line == "");
    CHECK(in.tellg() == 22);   // 3rd '\n'

    REQUIRE(GetlineBackwards::getline(in, line));
    CHECK(line == "third line");
    CHECK(in.tellg() == 11);   // 2nd '\n'

    REQUIRE(GetlineBackwards::getline(in, line));
    CHECK(line == "");
    CHECK(in.tellg() == 10);   // 1st '\n'

    REQUIRE(GetlineBackwards::getline(in, line));
    CHECK(line == "first line");
    CHECK(in.tellg() == 0);

    REQUIRE(!GetlineBackwards::getline(in, line));
}

TEST_CASE("GetlineBackwards: delimiter") {
    std::istringstream in("first\nline#second\nline");
    in.seekg(0, std::ios::end);

    std::string line;
    REQUIRE(GetlineBackwards::getline(in, line, '#'));
    CHECK(line == "second\nline");
}