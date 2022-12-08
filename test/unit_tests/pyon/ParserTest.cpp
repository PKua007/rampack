//
// Created by pkua on 07.12.22.
//

#include <catch2/catch.hpp>

#include "pyon/Parser.h"

using namespace pyon;
using namespace pyon::ast;
using namespace Catch;


TEST_CASE("Parser: literals") {
    SECTION("int") {
        SECTION("dec") {
            CHECK(Parser::parse("-45")->as<NodeInt>()->getValue() == -45);
        }

        SECTION("hex") {
            CHECK(Parser::parse("0xFF")->as<NodeInt>()->getValue() == 0xFF);
        }

        SECTION("oct") {
            CHECK(Parser::parse("01234")->as<NodeInt>()->getValue() == 01234);
        }

        SECTION("bin") {
            CHECK(Parser::parse("0b1001")->as<NodeInt>()->getValue() == 0b1001);
        }

        SECTION("error: overflow") {
            CHECK_THROWS_WITH(Parser::parse("123111111111111111111111"),
                              Contains(">>>>>>123") && Contains("out of range"));
        }
    }

    SECTION("float") {
        SECTION("normal") {
            CHECK(Parser::parse("-1.2e-4")->as<NodeFloat>()->getValue() == -1.2e-4);
        }

        SECTION("error: overflow") {
            CHECK_THROWS_WITH(Parser::parse("1.2e1234"), Contains(">>>>>>1.2") && Contains("out of range"));
        }

        SECTION("error: unsupported nan and inf") {
            CHECK_THROWS_WITH(Parser::parse("+inf"), Contains(">>>>>>+inf") && Contains("NaN/inf is not supported"));
            CHECK_THROWS_WITH(Parser::parse("+nan"), Contains(">>>>>>+nan") && Contains("NaN/inf is not supported"));
        }
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
                CHECK_THROWS_WITH(Parser::parse(R"("abc)"), Contains(">>>>>>\n") && Contains("unexpected EOF"));
            }

            SECTION("EOF after backslash") {
                CHECK_THROWS_WITH(Parser::parse(R"("abc\)"), Contains(">>>>>>\n") && Contains("unexpected EOF"));
            }

            SECTION("unknown escaped char") {
                CHECK_THROWS_WITH(Parser::parse(R"("abc\x")"),
                                  Contains(">>>>>>\\x") && Contains("'x' cannot follow backslash"));
            }
        }
    }

    SECTION("error: extra characters") {
        CHECK_THROWS_WITH(Parser::parse("1 a"), Contains(">>>>>>a") && Contains("unexpected character"));
        CHECK_THROWS_WITH(Parser::parse("0b"), Contains(">>>>>>b") && Contains("unexpected character"));
        CHECK_THROWS_WITH(Parser::parse("0x"), Contains(">>>>>>x") && Contains("unexpected character"));
        CHECK_THROWS_WITH(Parser::parse("1.2e-4."), Contains(">>>>>>.\n") && Contains("unexpected character"));
        CHECK_THROWS_WITH(Parser::parse("True,"),  Contains(">>>>>>,") && Contains("unexpected character"));
        CHECK_THROWS_WITH(Parser::parse(R"("abc" "")"), Contains(">>>>>>\"\"") && Contains("unexpected character"));
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
            CHECK_THROWS_WITH(Parser::parse("[1, 2, 3"), Contains(">>>>>>\n") && Contains("unexpected EOF"));
        }

        SECTION("misplaced comma") {
            CHECK_THROWS_WITH(Parser::parse("[, 2, 3]"), Contains(">>>>>>, 2") && Contains("unexpected character"));
        }

        SECTION("lack of comma") {
            CHECK_THROWS_WITH(Parser::parse("[1 2, 3]"), Contains(">>>>>>2") && Contains("missing comma"));
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
            CHECK_THROWS_WITH(Parser::parse(R"({"a" : 1, "b" : 1.2)"),
                              Contains(">>>>>>\n") && Contains("unexpected EOF"));
        }

        SECTION("misplaced comma") {
            CHECK_THROWS_WITH(Parser::parse(R"({"a" : 1, , "b" : 1.2})"),
                              Contains(">>>>>>, \"b") && Contains("expecting string"));
        }

        SECTION("lack of comma") {
            CHECK_THROWS_WITH(Parser::parse(R"({"a" : 1 "b" : 1.2})"),
                              Contains(">>>>>>\"b") && Contains("missing comma"));
        }

        SECTION("misplaced colon") {
            CHECK_THROWS_WITH(Parser::parse(R"({"a" : 1 : , "b" : 1.2})"),
                              Contains(">>>>>>: ,") && Contains("missing comma"));
        }

        SECTION("lack of colon") {
            CHECK_THROWS_WITH(Parser::parse(R"({"a" 1 , "b" : 1.2})"),
                              Contains(">>>>>>1 ,") && Contains("missing colon"));
        }

        SECTION("key not string") {
            CHECK_THROWS_WITH(Parser::parse(R"({1.2 : 1 , "b" : 1.2})"),
                              Contains(">>>>>>1.2") && Contains("expecting string"));
        }
    }
}

TEST_CASE("Parser: dataclass") {
    SECTION("empty") {
        auto node = Parser::parse("class");
        auto nodeDataclass = node->as<NodeDataclass>();
        CHECK(nodeDataclass->getClassName() == "class");
        CHECK(nodeDataclass->empty());
    }

    SECTION("correct names") {
        CHECK(Parser::parse("_class")->as<NodeDataclass>()->getClassName() == "_class");
        CHECK(Parser::parse("_")->as<NodeDataclass>()->getClassName() == "_");
        CHECK(Parser::parse("class0")->as<NodeDataclass>()->getClassName() == "class0");
        CHECK(Parser::parse("class_0")->as<NodeDataclass>()->getClassName() == "class_0");
    }

    SECTION("empty with parentheses") {
        auto node = Parser::parse("class()");
        auto nodeDataclass = node->as<NodeDataclass>();
        CHECK(nodeDataclass->getClassName() == "class");
        CHECK(nodeDataclass->empty());
    }

    SECTION("one positional argument") {
        auto node = Parser::parse("class(True)");
        auto nodeDataclass = node->as<NodeDataclass>();
        CHECK(nodeDataclass->getClassName() == "class");
        auto positional = nodeDataclass->getPositionalArguments();
        REQUIRE(positional->size() == 1);
        CHECK(positional->front()->as<NodeBoolean>()->getValue());
        CHECK(nodeDataclass->getKeywordArguments()->empty());
    }

    SECTION("two positional arguments") {
        auto node = Parser::parse("class(True, 1.2)");
        auto nodeDataclass = node->as<NodeDataclass>();
        CHECK(nodeDataclass->getClassName() == "class");
        auto positional = nodeDataclass->getPositionalArguments();
        REQUIRE(positional->size() == 2);
        CHECK(positional->at(0)->as<NodeBoolean>()->getValue());
        CHECK(positional->at(1)->as<NodeFloat>()->getValue() == 1.2);
        CHECK(nodeDataclass->getKeywordArguments()->empty());
    }

    SECTION("two keyword arguments") {
        auto node = Parser::parse("class(a=True, b=1.2)");
        auto nodeDataclass = node->as<NodeDataclass>();
        CHECK(nodeDataclass->getClassName() == "class");
        CHECK(nodeDataclass->getPositionalArguments()->empty());
        auto keyword = nodeDataclass->getKeywordArguments();
        REQUIRE(keyword->size() == 2);
        CHECK(keyword->at("a")->as<NodeBoolean>()->getValue());
        CHECK(keyword->at("b")->as<NodeFloat>()->getValue() == 1.2);
    }

    SECTION("mixed arguments") {
        auto node = Parser::parse("class(True, b=1.2)");
        auto nodeDataclass = node->as<NodeDataclass>();
        CHECK(nodeDataclass->getClassName() == "class");
        auto positional = nodeDataclass->getPositionalArguments();
        REQUIRE(positional->size() == 1);
        CHECK(positional->front()->as<NodeBoolean>()->getValue());
        auto keyword = nodeDataclass->getKeywordArguments();
        REQUIRE(keyword->size() == 1);
        CHECK(keyword->at("b")->as<NodeFloat>()->getValue() == 1.2);
    }

    SECTION("errors") {
        SECTION("unmatched '('") {
            CHECK_THROWS_WITH(Parser::parse("class(3, a=7"), Contains(">>>>>>\n") && Contains("unexpected EOF"));
        }

        SECTION("misplaced comma") {
            CHECK_THROWS_WITH(Parser::parse("class(3, ,a=7)"),
                              Contains(">>>>>>,a") && Contains("unexpected character"));
        }

        SECTION("missing comma") {
            CHECK_THROWS_WITH(Parser::parse("class(3 a=7)"), Contains(">>>>>>a=7") && Contains("missing comma"));
        }

        SECTION("extra characters") {
            CHECK_THROWS_WITH(Parser::parse("class(1.2extra, 4)"),
                              Contains(">>>>>>extra") && Contains("missing comma"));
        }

        SECTION("missing keyword value") {
            CHECK_THROWS_WITH(Parser::parse("class(a=, 3)"), Contains(">>>>>>, ") && Contains("unexpected character"));
        }

        SECTION("positional after keyword") {
            CHECK_THROWS_WITH(Parser::parse("class(3, a=True, None)"), Contains(">>>>>>None")
                              && Contains("positional arguments cannot follow keyword arguments"));
        }
    }
}

TEST_CASE("Parser: everything at once") {
    auto tree = Parser::parse(R"([
        class1({"a" : {"aa" : 1, "ab" : 2}, "b" : 3}, keyword=None),
        ["abc", "def"],
        1.2e-4,
        class2(b=[True, False])
    ])");

    auto rootArray = tree->as<NodeArray>();
    REQUIRE(rootArray->size() == 4);

    auto array0Class = rootArray->at(0)->as<NodeDataclass>();
    CHECK(array0Class->getClassName() == "class1");
    auto array0ClassPos = array0Class->getPositionalArguments();
    REQUIRE(array0ClassPos->size() == 1);
    auto array0ClassPosDict = array0ClassPos->at(0)->as<NodeDictionary>();
    REQUIRE(array0ClassPosDict->size() == 2);
    auto array0ClassPosDictADict = array0ClassPosDict->at("a")->as<NodeDictionary>();
    REQUIRE(array0ClassPosDictADict->size() == 2);
    CHECK(array0ClassPosDictADict->at("aa")->as<NodeInt>()->getValue() == 1);
    CHECK(array0ClassPosDictADict->at("ab")->as<NodeInt>()->getValue() == 2);
    CHECK(array0ClassPosDict->at("b")->as<NodeInt>()->getValue() == 3);
    auto array0ClassKey = array0Class->getKeywordArguments();
    REQUIRE(array0ClassKey->size() == 1);
    CHECK(array0ClassKey->at("keyword")->getType() == Node::NONE);

    auto array1Array = rootArray->at(1)->as<NodeArray>();
    REQUIRE(array1Array->size() == 2);
    CHECK(array1Array->at(0)->as<NodeString>()->getValue() == "abc");
    CHECK(array1Array->at(1)->as<NodeString>()->getValue() == "def");

    auto array2Float = rootArray->at(2)->as<NodeFloat>();
    CHECK(array2Float->getValue() == 1.2e-4);

    auto array3Class = rootArray->at(3)->as<NodeDataclass>();
    CHECK(array3Class->getClassName() == "class2");
    auto array3ClassKey = array3Class->getKeywordArguments();
    REQUIRE(array0ClassKey->size() == 1);
    auto array3ClassKeyBArray = array3ClassKey->at("b")->as<NodeArray>();
    REQUIRE(array3ClassKeyBArray->size() == 2);
    CHECK(array3ClassKeyBArray->at(0)->as<NodeBoolean>()->getValue());
    CHECK_FALSE(array3ClassKeyBArray->at(1)->as<NodeBoolean>()->getValue());
}

TEST_CASE("Parser: whitespaces") {
    auto input = R"( [
        class1  (  {  "a"  :  {  "aa"  :  1  ,  "ab"  :  2 }  ,  "b"  :  3 }  ,  keyword  =  None  )  ,
        [  "abc"  ,  "def"  ]  ,
        1.2e-4  ,
        class2  (  b  =  [  True  ,  False  ]  )
    ]
    )";

    CHECK_NOTHROW(Parser::parse(input));
}
