//
// Created by pkua on 22.05.22.
//

#include <catch2/catch.hpp>

#include "core/lattice/SerialPopulator.h"


TEST_CASE("SerialPopulator: single shape in cell") {
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0.5, 0.5, 0.5})}), {2, 2, 2});

    SECTION("default start") {
        SerialPopulator serialPopulator("yxz");

        auto shapes = serialPopulator.populateLattice(lattice, 3);

        CHECK(shapes == std::vector<Shape>{Shape({0.5, 0.5, 0.5}), Shape({0.5, 0.5, 1.5}), Shape({1.5, 0.5, 0.5})});
    }

    SECTION("delayed start") {
        SerialPopulator serialPopulator("yxz", 2);

        auto shapes = serialPopulator.populateLattice(lattice, 3);

        CHECK(shapes == std::vector<Shape>{Shape({1.5, 0.5, 0.5}), Shape({1.5, 0.5, 1.5}), Shape({0.5, 1.5, 0.5})});
    }

    SECTION("delayed start and skip") {
        SerialPopulator serialPopulator("yxz", 1, 3);

        auto shapes = serialPopulator.populateLattice(lattice, 3);

        CHECK(shapes == std::vector<Shape>{Shape({0.5, 0.5, 1.5}), Shape({0.5, 1.5, 0.5}), Shape({1.5, 1.5, 1.5})});
    }

    SECTION("fill all with delayed start and skip") {
        SerialPopulator serialPopulator("yxz", 1, 3);

        auto shapes = serialPopulator.populateLattice(lattice, std::nullopt);

        CHECK(shapes == std::vector<Shape>{Shape({0.5, 0.5, 1.5}), Shape({0.5, 1.5, 0.5}), Shape({1.5, 1.5, 1.5})});
    }
}

TEST_CASE("SerialPopulator: 2 shapes in cell") {
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0.5, 0.5, 0.5}), Shape({0.5, 0.5, 0.25})}), {2, 2, 2});

    SECTION("default start") {
        SerialPopulator serialPopulator("yxz");

        auto shapes = serialPopulator.populateLattice(lattice, 5);

        CHECK(shapes == std::vector<Shape>{Shape({0.5, 0.5, 0.5}), Shape({0.5, 0.5, 0.25}),
                                           Shape({0.5, 0.5, 1.5}), Shape({0.5, 0.5, 1.25}),
                                           Shape({1.5, 0.5, 0.5})});
    }

    SECTION("delayed start") {
        SerialPopulator serialPopulator("yxz", 1);

        auto shapes = serialPopulator.populateLattice(lattice, 3);

        CHECK(shapes == std::vector<Shape>{Shape({0.5, 0.5, 0.25}), Shape({0.5, 0.5, 1.5}), Shape({0.5, 0.5, 1.25})});
    }

    SECTION("delayed start and skip") {
        SerialPopulator serialPopulator("yxz", 1, 2);

        auto shapes = serialPopulator.populateLattice(lattice, 2);

        CHECK(shapes == std::vector<Shape>{Shape({0.5, 0.5, 0.25}), Shape({0.5, 0.5, 1.25})});
    }

    SECTION("fill all with delayed start and skip") {
        SerialPopulator serialPopulator("yxz", 1, 2);

        auto shapes = serialPopulator.populateLattice(lattice, std::nullopt);

        CHECK(shapes == std::vector<Shape>{Shape({0.5, 0.5, 0.25}), Shape({0.5, 0.5, 1.25}),
                                           Shape({1.5, 0.5, 0.25}), Shape({1.5, 0.5, 1.25}),
                                           Shape({0.5, 1.5, 0.25}), Shape({0.5, 1.5, 1.25}),
                                           Shape({1.5, 1.5, 0.25}), Shape({1.5, 1.5, 1.25})});
    }
}

TEST_CASE("SerialPopulator: errors") {
    Lattice lattice(UnitCell(TriclinicBox(1), {Shape({0.5, 0.5, 0.5})}), {2, 2, 2});

    SECTION("no shapes") {
        CHECK_THROWS_AS(SerialPopulator("xyz").populateLattice(lattice, 0), PreconditionException);
    }

    SECTION("too many shapes") {
        CHECK_NOTHROW(SerialPopulator("xyz").populateLattice(lattice, 8));
        CHECK_THROWS_AS(SerialPopulator("xyz").populateLattice(lattice, 9), PreconditionException);

        CHECK_NOTHROW(SerialPopulator("xyz", 1).populateLattice(lattice, 7));
        CHECK_THROWS_AS(SerialPopulator("xyz", 2).populateLattice(lattice, 7), PreconditionException);

        CHECK_NOTHROW(SerialPopulator("xyz", 1, 3).populateLattice(lattice, 3));
        CHECK_THROWS_AS(SerialPopulator("xyz", 2, 3).populateLattice(lattice, 3), PreconditionException);
    }

    SECTION("startFrom too far") {
        CHECK_NOTHROW(SerialPopulator("xyz", 7).populateLattice(lattice, std::nullopt));
        CHECK_THROWS_AS(SerialPopulator("xyz", 8).populateLattice(lattice, std::nullopt), PreconditionException);
    }
}
