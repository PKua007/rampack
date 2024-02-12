//
// Created by Piotr Kubala on 12/02/2024.
//

#include <catch2/catch.hpp>

#include "core/io/ShapeDataSerializer.h"


TEST_CASE("ShapeDataSerializer") {
    ShapeDataSerializer serializer;

    serializer["double"] = 2.125;
    serializer["string"] = "abc";
    serializer["int"] = 10;
    serializer["vector"] = Vector<3>{1, 2, 3};

    auto data = serializer.toTextualShapeData();
    CHECK(data.size() == 4);
    CHECK(data.at("double") == "2.125");
    CHECK(data.at("string") == "abc");
    CHECK(data.at("int") == "10");
    CHECK(data.at("vector") == "1,2,3");
}