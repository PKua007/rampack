//
// Created by Piotr Kubala on 10/02/2020.
//

#include <catch2/catch.hpp>
#include <sstream>

#include "utils/Config.h"

TEST_CASE("Config: parsing sections tree") {
    std::stringstream in;
    in << "k1 = 1" << std::endl;
    in << "[s1]" << std::endl;
    in << "k2 = 2" << std::endl;
    in << "[s2]" << std::endl;
    in << "k3 = 3" << std::endl;
    in << "[s2.ss1]" << std::endl;
    in << "k4 = 4" << std::endl;
    in << "[s2.ss2]" << std::endl;
    in << "k5 = 5" << std::endl;

    SECTION("root") {
        auto config = Config::parse(in);

        auto expected = std::vector<std::string>{"k1", "s1.k2", "s2.k3", "s2.ss1.k4", "s2.ss2.k5"};
        REQUIRE_THAT(config.getKeys(), Catch::UnorderedEquals(expected));
        expected = {"", "s1", "s2"};
        REQUIRE_THAT(config.getRootSections(), Catch::UnorderedEquals(expected));
        REQUIRE(config.getString("k1") == "1");
        REQUIRE(config.getString("s1.k2") == "2");
        REQUIRE(config.getString("s2.k3") == "3");
        REQUIRE(config.getString("s2.ss1.k4") == "4");
        REQUIRE(config.getString("s2.ss2.k5") == "5");

        SECTION("first level") {
            auto rootconfig = config.fetchSubconfig("");

            expected = {""};
            REQUIRE_THAT(rootconfig.getRootSections(), Catch::UnorderedEquals(expected));
            expected = {"k1"};
            REQUIRE_THAT(rootconfig.getKeys(), Catch::UnorderedEquals(expected));
            REQUIRE(rootconfig.getString("k1") == "1");

            auto subconfig1 = config.fetchSubconfig("s1");

            expected = {""};
            REQUIRE_THAT(subconfig1.getRootSections(), Catch::UnorderedEquals(expected));
            expected = {"k2"};
            REQUIRE_THAT(subconfig1.getKeys(), Catch::UnorderedEquals(expected));
            REQUIRE(subconfig1.getString("k2") == "2");

            auto subconfig2 = config.fetchSubconfig("s2");

            expected = {"", "ss1", "ss2"};
            REQUIRE_THAT(subconfig2.getRootSections(), Catch::UnorderedEquals(expected));
            expected = {"k3", "ss1.k4", "ss2.k5"};
            REQUIRE_THAT(subconfig2.getKeys(), Catch::UnorderedEquals(expected));
            REQUIRE(subconfig2.getString("k3") == "3");
            REQUIRE(subconfig2.getString("ss1.k4") == "4");
            REQUIRE(subconfig2.getString("ss2.k5") == "5");

            SECTION("second level") {
                auto subconfig3 = subconfig2.fetchSubconfig("ss1");

                expected = {""};
                REQUIRE_THAT(subconfig3.getRootSections(), Catch::UnorderedEquals(expected));
                expected = {"k4"};
                REQUIRE_THAT(subconfig3.getKeys(), Catch::UnorderedEquals(expected));
                REQUIRE(subconfig3.getString("k4") == "4");

                auto subconfig4 = subconfig2.fetchSubconfig("ss1");

                expected = {""};
                REQUIRE_THAT(subconfig4.getRootSections(), Catch::UnorderedEquals(expected));
                expected = {"k4"};
                REQUIRE_THAT(subconfig4.getKeys(), Catch::UnorderedEquals(expected));
                REQUIRE(subconfig4.getString("k4") == "4");
            }
        }
    }
}

TEST_CASE("Config: line continuation") {
    SECTION("correct") {
        std::stringstream in;
        in << "k1 = 1" << std::endl;
        in << "k2 = 2 \\" << std::endl;
        in << "continued" << std::endl;
        in << "k3 = 3 \\  # comment" << std::endl;
        in << "    " << std::endl;
        in << " this = looks like key \\" << std::endl;
        in << "the end  " << std::endl;
        in << "k4 = 4" << std::endl;

        auto config = Config::parse(in);

        REQUIRE_THAT(config.getKeys(), Catch::UnorderedEquals(std::vector<std::string>{"k1", "k2", "k3", "k4"}));
        CHECK(config.getString("k1") == "1");
        CHECK(config.getString("k2") == "2\ncontinued");
        CHECK(config.getString("k3") == "3\nthis = looks like key\nthe end");
        CHECK(config.getString("k4") == "4");
    }

    SECTION("unexpected end") {
        std::stringstream in;
        in << "k1 = 1" << std::endl;
        in << "k2 = 2 \\" << std::endl;
        in << "    # comment" << std::endl;
        in << "  " << std::endl;

        CHECK_THROWS_WITH(Config::parse(in), Catch::Contains("unexpected end", Catch::CaseSensitive::No));
    }
}