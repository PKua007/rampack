//
// Created by Piotr Kubala on 26/12/2020.
//

#include <catch2/catch.hpp>

#include "core/NeighbourGrid.h"

namespace {
    auto make_vector(NeighbourGrid::CellView cellView) {
        return std::vector<std::size_t>(cellView.begin(), cellView.end());
    }
}

TEST_CASE("NeighbourGrid") {
    // This is how the neighbour grid looks like (the middle slice - we ignore z dimension)
    //
    // +-------+-------+-------+-------+-------+
    // |       |       |       |       |       |
    // |       |   4   |       |       |   5   |
    // +-------+-------+-------+-------+-------+
    // |       |   0   |       |       |       |
    // |       | 1     |       |       |       |
    // +-------+-------+-------+-------+-------+
    // |       |       |    2  |       |       |
    // |       |       |       |       |       |
    // +-------+-------+-------+-------+-------+
    // | 3     |       |       |       |       |
    // |       |       |       |       |       |
    // +-------+-------+-------+-------+-------+

    // We will test the same neighbour list, but with 4 scenarios: without resizing, resizing down, up and to the same
    // size
    std::string testMode = GENERATE("without resizing", "from 7.5 up", "the same size", "from 12.5 down");
    std::array<double, 3> linearSize{};
    if (testMode == "without resizing")     linearSize = {13, 10, 10};
    else if (testMode == "from 7.5 up")     linearSize = {9.75, 7.5, 7.5};
    else if (testMode == "the same size")   linearSize = {13, 10, 10};
    else if (testMode == "from 12.5 down")  linearSize = {16.25, 12.5, 10};
    else FAIL("test error");

    // Cell size 2.4 is not a "divisor" of 10, nor 13, so it should be corrected to 2.5 and 2.6
    NeighbourGrid neighbourGrid(linearSize, 2.4, 7);

    DYNAMIC_SECTION(testMode) {
        if (testMode != "without resizing")
            neighbourGrid.resize({13, 10, 10}, 2.4);

        neighbourGrid.add(0, {4, 7.49, 3}); // 7.49 - very close to the edge of a cell
        neighbourGrid.add(1, {3, 5.5, 3});
        neighbourGrid.add(2, {7, 4, 3});
        neighbourGrid.add(3, {1, 2, 3});
        neighbourGrid.add(4, {4, 9, 3});
        neighbourGrid.add(5, {12, 9, 3});

        SECTION("non-reflected neighbours") {
            SECTION("getNeighbours") {
                auto neighbours = neighbourGrid.getNeighbours({3, 7, 3});

                REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{0, 1, 2, 4}));
            }

            SECTION("iterator") {
                std::vector<std::size_t> neighbours;

                for (const auto &cell : neighbourGrid.getNeighbouringCells(Vector<3>{3, 7, 3}))
                    for (const auto &neighbour : cell.getNeighbours())
                        neighbours.push_back(neighbour);

                REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{0, 1, 2, 4}));
            }
        }

        SECTION("reflected neighbours") {
            SECTION("getNeighbours") {
                auto neighbours = neighbourGrid.getNeighbours({11, 9, 3});

                REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{3, 5}));
            }

            SECTION("iterator") {
                std::vector<std::size_t> neighbours;

                for (const auto &cell : neighbourGrid.getNeighbouringCells(Vector<3>{11, 9, 3}))
                    for (const auto &neighbour : cell.getNeighbours())
                        neighbours.push_back(neighbour);

                REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{3, 5}));
            }
        }

        SECTION("getSpecificCell") {
            REQUIRE_THAT(make_vector(neighbourGrid.getCell(Vector<3>{3, 7, 3})),
                         Catch::UnorderedEquals(std::vector<std::size_t>{0, 1}));
            REQUIRE(make_vector(neighbourGrid.getCell(Vector<3>{6, 1, 3})).empty());
        }

        SECTION("add") {
            neighbourGrid.add(6, {6, 4, 3});
            auto neighbours = neighbourGrid.getNeighbours({3, 7, 3});

            REQUIRE_THAT(make_vector(neighbourGrid.getCell(Vector<3>{6, 4.5, 3})),
                         Catch::UnorderedEquals(std::vector<std::size_t>{2, 6}));
            REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{0, 1, 2, 4, 6}));
        }

        SECTION("remove existing") {
            SECTION("first from two") {
                neighbourGrid.remove(0, {3, 7, 3});
                auto neighbours = neighbourGrid.getNeighbours({3, 7, 3});

                REQUIRE(make_vector(neighbourGrid.getCell(Vector<3>{3.5, 6.5, 3})) == std::vector<std::size_t>{1});
                REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{1, 2, 4}));
            }

            SECTION("second from two") {
                neighbourGrid.remove(1, {3, 7, 3});
                auto neighbours = neighbourGrid.getNeighbours({3, 7, 3});

                REQUIRE(make_vector(neighbourGrid.getCell(Vector<3>{3.5, 6.5, 3})) == std::vector<std::size_t>{0});
                REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{0, 2, 4}));
            }

            SECTION("both from two") {
                neighbourGrid.remove(0, {3, 7, 3});
                neighbourGrid.remove(1, {3, 7, 3});
                auto neighbours = neighbourGrid.getNeighbours({3, 7, 3});

                REQUIRE(make_vector(neighbourGrid.getCell(Vector<3>{3.5, 6.5, 3})).empty());
                REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{2, 4}));
            }

            SECTION("both from two: different order") {
                neighbourGrid.remove(1, {3, 7, 3});
                neighbourGrid.remove(0, {3, 7, 3});
                auto neighbours = neighbourGrid.getNeighbours({3, 7, 3});

                REQUIRE(make_vector(neighbourGrid.getCell(Vector<3>{3.5, 6.5, 3})).empty());
                REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{2, 4}));
            }
        }

        SECTION("remove nonexistent") {
            neighbourGrid.remove(2, {3, 7, 3});
            auto neighbours = neighbourGrid.getNeighbours({3, 7, 3});

            REQUIRE_THAT(make_vector(neighbourGrid.getCell(Vector<3>{3.5, 6.5, 3})),
                         Catch::UnorderedEquals(std::vector<std::size_t>{0, 1}));
            REQUIRE_THAT(neighbours, Catch::UnorderedEquals(std::vector<std::size_t>{0, 1, 2, 4}));
        }

        SECTION("clearing") {
            neighbourGrid.clear();

            REQUIRE(make_vector(neighbourGrid.getCell(Vector<3>{0.5, 0.5, 3})).empty());
            REQUIRE(make_vector(neighbourGrid.getCell(Vector<3>{5.5, 5.5, 3})).empty());
            REQUIRE(make_vector(neighbourGrid.getCell(Vector<3>{9.5, 4.5, 3})).empty());
            REQUIRE(make_vector(neighbourGrid.getCell(Vector<3>{0.5, 9.8, 3})).empty());
            REQUIRE(neighbourGrid.getNeighbours({3, 7, 3}).empty());
            REQUIRE(neighbourGrid.getNeighbours({2, 5, 3}).empty());
            REQUIRE(neighbourGrid.getNeighbours({5, 9, 3}).empty());
        }

        SECTION("swap") {
            NeighbourGrid neighbourGrid2(linearSize, 2.4, 7);

            std::swap(neighbourGrid, neighbourGrid2);

            REQUIRE_THAT(neighbourGrid2.getNeighbours({3, 7, 3}),
                         Catch::UnorderedEquals(std::vector<std::size_t>{0, 1, 2, 4}));
        }
    }
}
