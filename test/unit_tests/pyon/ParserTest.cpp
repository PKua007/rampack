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

TEST_CASE("Parser: array") {
    SECTION("empty") {
        auto node = Parser::parse("[]");
        CHECK(node->as<NodeArray>()->empty());
    }

    SECTION("one element") {
        auto node = Parser::parse("[5]");
        auto nodeArray = node->as<NodeArray>();
        REQUIRE(nodeArray->size() == 1);
        CHECK(nodeArray->at(0)->as<NodeInt>()->getValue() == 5);
    }

    SECTION("three elements") {
        auto node = Parser::parse(R"([5, 1.2, "abc"])");
        auto nodeArray = node->as<NodeArray>();
        REQUIRE(nodeArray->size() == 3);
        CHECK(nodeArray->at(0)->as<NodeInt>()->getValue() == 5);
        CHECK(nodeArray->at(1)->as<NodeFloat>()->getValue() == 1.2);
        CHECK(nodeArray->at(2)->as<NodeString>()->getValue() == "abc");
    }

    SECTION("nested") {
        auto node = Parser::parse("[[1, 2, 3], [4, 5, 6], [7, 8, 9]]");
        auto nodeArray = node->as<NodeArray>();
        REQUIRE(nodeArray->size() == 3);
        for (std::size_t i{}; i < 3; i++) {
            auto nodeNested = nodeArray->at(i)->as<NodeArray>();
            REQUIRE(nodeNested->size() == 3);
            for (std::size_t j{}; j < 3; j++)
                CHECK(nodeNested->at(j)->as<NodeInt>()->getValue() == static_cast<long int>(3*i + j + 1));
        }
    }

    SECTION("errors") {
        SECTION("lacking ']'") {
            CHECK_THROWS_AS(Parser::parse("[1, 2, 3"), ParseException);
        }

        SECTION("misplaced comma") {
            CHECK_THROWS_AS(Parser::parse("[, 2, 3]"), ParseException);
        }

        SECTION("lack of comma") {
            CHECK_THROWS_AS(Parser::parse("[1 2, 3]"), ParseException);
        }
    }
}

TEST_CASE("Parser: dictionary") {
    SECTION("empty") {
        auto node = Parser::parse("{}");
        CHECK(node->as<NodeDictionary>()->empty());
    }

    SECTION("one element") {
        auto node = Parser::parse(R"({"a" : 1})");
        auto nodeDict = node->as<NodeDictionary>();
        REQUIRE(nodeDict->size() == 1);
        CHECK(nodeDict->at("a")->as<NodeInt>()->getValue() == 1);
    }

    SECTION("three elements") {
        auto node = Parser::parse(R"({"a" : 1, "b" : 1.2, "c" : "abc"})");
        auto nodeDict = node->as<NodeDictionary>();
        REQUIRE(nodeDict->size() == 3);
        CHECK(nodeDict->at("a")->as<NodeInt>()->getValue() == 1);
        CHECK(nodeDict->at("b")->as<NodeFloat>()->getValue() == 1.2);
        CHECK(nodeDict->at("c")->as<NodeString>()->getValue() == "abc");
    }

    SECTION("errors") {
        SECTION("lacking '}'") {
            CHECK_THROWS_AS(Parser::parse(R"({"a" : 1, "b" : 1.2)"), ParseException);
            Parser::parse(R"({"a" : 1, "b" : 1.2)");
        }

        SECTION("misplaced comma") {
            CHECK_THROWS_AS(Parser::parse(R"({"a" : 1, , "b" : 1.2})"), ParseException);
            Parser::parse(R"({"a" : 1, , "b" : 1.2})");
        }

        SECTION("lack of comma") {
            CHECK_THROWS_AS(Parser::parse(R"({"a" : 1 "b" : 1.2})"), ParseException);
            Parser::parse(R"({"a" : 1 "b" : 1.2})");
        }

        SECTION("misplaced colon") {
            CHECK_THROWS_AS(Parser::parse(R"({"a" : 1 : , "b" : 1.2})"), ParseException);
            Parser::parse(R"({"a" : 1 : , "b" : 1.2})");
        }

        SECTION("lack of colon") {
            CHECK_THROWS_AS(Parser::parse(R"({"a" 1 , "b" : 1.2})"), ParseException);
            Parser::parse(R"({"a" 1 , "b" : 1.2})");
        }

        SECTION("key not string") {
            CHECK_THROWS_AS(Parser::parse(R"({1.2 : 1 , "b" : 1.2})"), ParseException);
            Parser::parse(R"({1.2 : 1 , "b" : 1.2})");
        }
    }
}
