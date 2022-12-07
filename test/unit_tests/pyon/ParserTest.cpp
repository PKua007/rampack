//
// Created by pkua on 07.12.22.
//

#include <catch2/catch.hpp>

#include "pyon/Parser.h"

using namespace pyon;
using namespace pyon::ast;


TEST_CASE("Parser: literals") {
    SECTION("int") {
        auto node = Parser::parse("45");
        CHECK(node->as<NodeInt>()->getValue() == 45);
    }

    SECTION("float") {
        auto node = Parser::parse("1.2e-4");
        CHECK(node->as<NodeFloat>()->getValue() == 1.2e-4);
    }
}