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

    SECTION("boolean") {
        auto trueNode = Parser::parse("True");
        CHECK(trueNode->as<NodeBoolean>()->getValue());
        auto falseNode = Parser::parse("False");
        CHECK_FALSE(falseNode->as<NodeBoolean>()->getValue());
    }

    SECTION("none") {
        auto node = Parser::parse("None");
        CHECK(node->getType() == Node::NONE);
    }

    SECTION("string") {
        SECTION("empty") {
            auto node = Parser::parse(R"("")");
            CHECK(node->as<NodeString>()->getValue().empty());
        }

        SECTION("easy") {
            auto node = Parser::parse(R"("abc")");
            CHECK(node->as<NodeString>()->getValue() == "abc");
        }

        SECTION("escaped") {
            auto node = Parser::parse(R"("a\n\tb\\")");
            CHECK(node->as<NodeString>()->getValue() == "a\n\tb\\");
        }

        SECTION("errors") {
            SECTION("EOF") {
                CHECK_THROWS_AS(Parser::parse(R"("abc)"), ParseException);
            }

            SECTION("EOF after backslash") {
                CHECK_THROWS_AS(Parser::parse(R"("abc\)"), ParseException);
            }

            SECTION("unknown escaped char") {
                CHECK_THROWS_AS(Parser::parse(R"("abc\x")"), ParseException);
            }
        }
    }
}