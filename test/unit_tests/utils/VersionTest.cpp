//
// Created by pkua on 23.11.22.
//

#include <catch2/catch.hpp>

#include "utils/Version.h"


TEST_CASE("Version: default") {
    CHECK(Version{} == Version(0, 0, 0));
}

TEST_CASE("Version: string parsing") {
    SECTION("correct") {
        CHECK(Version{"1.2.3"} == Version(1, 2, 3));
        CHECK(Version{"1.2"} == Version(1, 2, 0));
        CHECK(Version{"1"} == Version(1, 0, 0));
    }

    SECTION("incorrect") {
        CHECK_THROWS(Version{""});
        CHECK_THROWS(Version{"1."});
        CHECK_THROWS(Version{"1..2"});
        CHECK_THROWS(Version{"1.2a.3"});
        CHECK_THROWS(Version{"1.a2.3"});
    }
}

TEST_CASE("Version: comparison") {
    CHECK(Version(1, 2, 3) == Version(1, 2, 3));
    CHECK(Version(1, 2, 3) != Version(1, 2, 4));

    CHECK(Version(1, 2, 3) < Version(2, 1, 1));
    CHECK(Version(1, 2, 3) < Version(1, 3, 1));

    CHECK(Version(1, 2, 3) < Version(1, 2, 4));
    CHECK_FALSE(Version(1, 2, 3) < Version(1, 2, 3));
    CHECK(Version(1, 2, 3) <= Version(1, 2, 4));
    CHECK(Version(1, 2, 3) <= Version(1, 2, 3));
    CHECK(Version(1, 2, 4) > Version(1, 2, 3));
    CHECK_FALSE(Version(1, 2, 3) > Version(1, 2, 3));
    CHECK(Version(1, 2, 4) >= Version(1, 2, 3));
    CHECK(Version(1, 2, 3) >= Version(1, 2, 3));
}