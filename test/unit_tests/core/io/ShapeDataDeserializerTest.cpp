//
// Created by Piotr Kubala on 12/02/2024.
//

#include <catch2/catch.hpp>

#include "core/io/ShapeDataDeserializer.h"

namespace {
    template<typename T>
    T deserializer_as(const std::string &value) {
        ShapeDataDeserializer deserializer({{"key", value}});
        return deserializer.as<T>("key");
    }
}


TEST_CASE("ShapeDataDeserializer: basic") {
    ShapeDataDeserializer deserializer({
        {"double", "2.125"},
        {"string", "abc"},
        {"int", "10"},
        {"vector", "1,2,3"}
    });

    SECTION("as") {
        CHECK(deserializer.as<double>("double") == 2.125);
        CHECK(deserializer.as<std::string>("string") == "abc");
        CHECK(deserializer.as<int>("int") == 10);
        CHECK(deserializer.as<Vector<3>>("vector") == Vector<3>{1, 2, 3});

        SECTION("check access") {
            CHECK(deserializer.wasAccessed("double"));
            CHECK(deserializer.wasAccessed("string"));
            CHECK(deserializer.wasAccessed("int"));
            CHECK(deserializer.wasAccessed("vector"));
            CHECK_NOTHROW(deserializer.throwIfNotAccessed());
        }
    }

    SECTION("not all accessed") {
        CHECK(deserializer.as<std::string>("string") == "abc");

        CHECK_FALSE(deserializer.wasAccessed("double"));
        CHECK(deserializer.wasAccessed("string"));
        CHECK_FALSE(deserializer.wasAccessed("int"));
        CHECK_FALSE(deserializer.wasAccessed("vector"));
        CHECK_THROWS_WITH(deserializer.throwIfNotAccessed(), "Unknown params were present: double, int, vector");
    }

    SECTION("hasParam") {
        CHECK(deserializer.hasParam("string"));
        CHECK_FALSE(deserializer.hasParam("not_existing"));
    }
}

TEST_CASE("ShapeDataDeserializer: errors") {
    using Catch::Contains;
    using Catch::CaseSensitive;

    SECTION("malformed") {
        CHECK_THROWS_WITH(deserializer_as<int>("a"), Contains("malformed", CaseSensitive::No));
        CHECK_THROWS_WITH(deserializer_as<unsigned int>("-5"), Contains("malformed", CaseSensitive::No));
        CHECK_THROWS_WITH(deserializer_as<double>("2.45y"), Contains("malformed", CaseSensitive::No));
        CHECK_THROWS_WITH(deserializer_as<Vector<3>>("1,2"), Contains("3 comma-separated", CaseSensitive::No));
        CHECK_THROWS_WITH(deserializer_as<Vector<3>>("1,2,3,4"), Contains("3 comma-separated", CaseSensitive::No));
        CHECK_THROWS_WITH(deserializer_as<Vector<3>>("1,a,3"), Contains("malformed", CaseSensitive::No));
    }

    SECTION("out of range") {
        CHECK_THROWS_WITH(deserializer_as<short>("1000000"), Contains("out of range", CaseSensitive::No));
        CHECK_THROWS_WITH(deserializer_as<double>("1e500"), Contains("out of range", CaseSensitive::No));
        CHECK_THROWS_WITH(deserializer_as<Vector<3>>("1,2,3e500"), Contains("out of range", CaseSensitive::No));
    }

    SECTION("no key") {
        ShapeDataDeserializer deserializer({{"key", "value"}});
        CHECK_THROWS_WITH(deserializer.as<int>("no_key"), Contains("unknown", CaseSensitive::No));
    }
}