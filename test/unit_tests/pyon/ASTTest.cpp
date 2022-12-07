//
// Created by pkua on 06.12.22.
//

#include <catch2/catch.hpp>

#include "pyon/AST.h"

using namespace pyon::ast;


TEST_CASE("AST: unsigned") {
    auto intNode = NodeInt::create(5);
    CHECK(intNode->getType() == Node::INT);
    CHECK(NodeInt::NODE_TYPE == Node::INT);
    CHECK(intNode->getValue() == 5);
}

TEST_CASE("AST: float") {
    auto floatNode = NodeFloat::create(1.2);
    CHECK(floatNode->getType() == Node::FLOAT);
    CHECK(NodeFloat::NODE_TYPE == Node::FLOAT);
    CHECK(floatNode->getValue() == 1.2);
}

TEST_CASE("AST: booleen") {
    auto booleanNode = NodeBoolean::create(true);
    CHECK(booleanNode->getType() == Node::BOOLEAN);
    CHECK(NodeBoolean::NODE_TYPE == Node::BOOLEAN);
    CHECK(booleanNode->getValue());
}

TEST_CASE("AST: string") {
    auto stringNode = NodeString::create("abc");
    CHECK(stringNode->getType() == Node::STRING);
    CHECK(NodeString::NODE_TYPE == Node::STRING);
    CHECK(stringNode->getValue() == "abc");
}

TEST_CASE("AST: array") {
    SECTION("empty") {
        auto array = NodeArray::create();
        CHECK(array->getType() == Node::ARRAY);
        CHECK(NodeArray::NODE_TYPE == Node::ARRAY);
        CHECK(array->empty());
        CHECK(array->size() == 0);
        CHECK_THROWS_AS(array->front(), ArrayNoSuchIndexException);
        CHECK_THROWS_AS(array->back(), ArrayNoSuchIndexException);
        CHECK_THROWS_AS(array->at(0), ArrayNoSuchIndexException);
    }

    SECTION("non-empty") {
        auto array = NodeArray::create({NodeInt::create(-1), NodeString::create("abc")});
        CHECK_FALSE(array->empty());
        CHECK(array->size() == 2);
        CHECK(array->front() == array->at(0));
        CHECK(array->back() == array->at(1));
        CHECK(array->at(0)->as<NodeInt>()->getValue() == -1);
        CHECK(array->at(1)->as<NodeString>()->getValue() == "abc");
    }
}

TEST_CASE("AST: dictionary") {
    SECTION("empty") {
        auto dict = NodeDictionary::create();
        CHECK(dict->getType() == Node::DICTIONARY);
        CHECK(NodeDictionary::NODE_TYPE == Node::DICTIONARY);
        CHECK(dict->empty());
        CHECK(dict->size() == 0);
        CHECK_THROWS_AS(dict->at("a"), DictionaryNoSuchKeyException);
    }

    SECTION("non-empty") {
        auto dict = NodeDictionary::create({
                                               {"b", NodeInt::create(-1)},
                                               {"a", NodeString::create("abc")}
                                           });
        CHECK_FALSE(dict->empty());
        CHECK(dict->size() == 2);
        CHECK(dict->at("b")->as<NodeInt>()->getValue() == -1);
        CHECK(dict->at("a")->as<NodeString>()->getValue() == "abc");
        CHECK(dict->hasKey("b"));
        CHECK_FALSE(dict->hasKey("c"));
    }
}

TEST_CASE("AST: dataclass") {
    SECTION("empty") {
        auto dataclass = NodeDataclass::create("class", NodeArray::create(), NodeDictionary::create());
        CHECK(dataclass->getType() == Node::DATACLASS);
        CHECK(NodeDataclass::NODE_TYPE == Node::DATACLASS);
        CHECK(dataclass->getClassName() == "class");
        CHECK(dataclass->empty());
    }

    SECTION("non-empty") {
        auto positionalArguments = NodeArray::create({NodeInt::create(-5), NodeFloat::create(1.2)});
        auto keywordArguments = NodeDictionary::create({{"arg", NodeInt::create(5)}});
        auto dataclass = NodeDataclass::create("class", positionalArguments, keywordArguments);
        CHECK_FALSE(dataclass->empty());
        CHECK(dataclass->getClassName() == "class");
        CHECK(dataclass->getPositionalArguments() == positionalArguments);
        CHECK(dataclass->getKeywordArguments() == keywordArguments);
    }
}