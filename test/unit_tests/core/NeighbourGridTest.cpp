//
// Created by Piotr Kubala on 26/12/2020.
//

#include <catch2/catch.hpp>

#include "core/NeighbourGrid.h"

TEST_CASE("NeighbourGrid") {
    // This is how the neighbour grid looks like (the middle slice - we ignore z dimension)
    //
    // +-------+-------+-------+-------+
    // |       |       |       |       |
    // |       |   4   |       |   5   |
    // +-------+-------+-------+-------+
    // |       |   0   |       |       |
    // |       | 1     |       |       |
    // +-------+-------+-------+-------+
    // |       |       |    2  |       |
    // |       |       |       |       |
    // +-------+-------+-------+-------+
    // | 3     |       |       |       |
    // |       |       |       |       |
    // +-------+-------+-------+-------+

    // linear size is not an integer multiple of cellSize, so it has to be adjusted
    NeighbourGrid neighbourGrid(10, 2.4);
    neighbourGrid.add(0, {4, 7.49, 3}); // 7.49 - very close to the edge of a cell
    neighbourGrid.add(1, {3, 5.5, 3});
    neighbourGrid.add(2, {7, 4, 3});
    neighbourGrid.add(3, {1, 2, 3});
    neighbourGrid.add(4, {4, 9, 3});
    neighbourGrid.add(5, {9, 9, 3});

    SECTION("non-reflected neighbours") {
        auto neighbours = neighbourGrid.getNeighbours({3, 7, 3});

        REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{0, 1, 2, 4}));
    }

    SECTION("reflected neighbours") {
        auto neighbours = neighbourGrid.getNeighbours({9, 9, 3});

        REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{3, 5}));
    }

    SECTION("getCell") {
        REQUIRE_THAT(neighbourGrid.getCell({3, 7, 3}), Catch::UnorderedEquals(std::vector<std::size_t>{0, 1}));
        REQUIRE(neighbourGrid.getCell({6, 1, 3}).empty());
    }

    SECTION("add") {
        neighbourGrid.add(6, {6, 4, 3});
        auto neighbours = neighbourGrid.getNeighbours({3, 7, 3});

        REQUIRE_THAT(neighbourGrid.getCell({6, 4.5, 3}), Catch::UnorderedEquals(std::vector<std::size_t>{2, 6}));
        REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{0, 1, 2, 4, 6}));
    }

    SECTION("remove existing") {
        neighbourGrid.remove(0, {3, 7, 3});
        auto neighbours = neighbourGrid.getNeighbours({3, 7, 3});

        REQUIRE_THAT(neighbourGrid.getCell({3.5, 6.5, 3}), Catch::UnorderedEquals(std::vector<std::size_t>{1}));
        REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{1, 2, 4}));
    }

    SECTION("remove nonexistent") {
        neighbourGrid.remove(2, {3, 7, 3});
        auto neighbours = neighbourGrid.getNeighbours({3, 7, 3});

        REQUIRE_THAT(neighbourGrid.getCell({3.5, 6.5, 3}), Catch::UnorderedEquals(std::vector<std::size_t>{0, 1}));
        REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{0, 1, 2, 4}));
    }

    SECTION("clearing") {
        neighbourGrid.clear();

        REQUIRE(neighbourGrid.getCell({0.5, 0.5, 3}).empty());
        REQUIRE(neighbourGrid.getCell({5.5, 5.5, 3}).empty());
        REQUIRE(neighbourGrid.getCell({9.5, 4.5, 3}).empty());
        REQUIRE(neighbourGrid.getCell({0.5, 9.8, 3}).empty());
        REQUIRE(neighbourGrid.getNeighbours({3, 7, 3}).empty());
        REQUIRE(neighbourGrid.getNeighbours({2, 5, 3}).empty());
        REQUIRE(neighbourGrid.getNeighbours({5, 9, 3}).empty());
    }
}