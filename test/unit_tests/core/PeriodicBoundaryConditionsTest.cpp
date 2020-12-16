//
// Created by Piotr Kubala on 13/12/2020.
//

#include <catch2/catch.hpp>

#include "core/PeriodicBoundaryConditions.h"

TEST_CASE("PeriodicBoundaryConditions: getCorrection") {
    SECTION("in bounds") {
        PeriodicBoundaryConditions pbc(2);
        Vector<3> position{0.5, 1, 1.5};

        auto corr = pbc.getCorrection(position);

        CHECK(corr == Vector<3>{0, 0, 0});
    }

    SECTION("out of bounds") {
        PeriodicBoundaryConditions pbc(2);
        Vector<3> position{-0.5, 1, 2.5};

        auto corr = pbc.getCorrection(position);

        CHECK(corr == Vector<3>{2, 0, -2});
    }

    SECTION("very out of bounds") {
        PeriodicBoundaryConditions pbc(2);
        Vector<3> position{-101, 200.5, 300.75};

        auto corr = pbc.getCorrection(position);

        CHECK(corr == Vector<3>{102, -200, -300});
    }
}

TEST_CASE("PeriodicBoundaryConditions: getTranslation and getDiscance2") {
    SECTION("no translation") {
        PeriodicBoundaryConditions pbc(2);
        Vector<3> position1{0.5, 1, 1.5};
        Vector<3> position2{1, 1, 0.75};

        auto trans = pbc.getTranslation(position1, position2);
        double distance2 = pbc.getDistance2(position1, position2);

        CHECK(trans == Vector<3>{0, 0, 0});
        CHECK(distance2 == Approx(13. / 16));
    }

    SECTION("some translation") {
        PeriodicBoundaryConditions pbc(2);
        Vector<3> position1{0.25, 1.5, 1};
        Vector<3> position2{1.75, 0, 1};

        auto trans = pbc.getTranslation(position1, position2);
        double distance2 = pbc.getDistance2(position1, position2);

        CHECK(trans == Vector<3>{-2, 2, 0});
        CHECK(distance2 == Approx(0.5));
    }

    SECTION("big translation") {
        PeriodicBoundaryConditions pbc(2);
        std::array<double, 3> position1{-100, -300, 100};
        std::array<double, 3> position2{101.5, -200.5, 50};

        auto trans = pbc.getTranslation(position1, position2);
        double distance2 = pbc.getDistance2(position1, position2);

        CHECK(trans == Vector<3>{-202, -100, 50});
        CHECK(distance2 == Approx(0.5));
    }
}

TEST_CASE("PeriodicBoundaryConditions: default size should be 1") {
    PeriodicBoundaryConditions pbc;
    Vector<3> position{0.5, -0.5, 1.5};

    auto corr = pbc.getCorrection(position);

    CHECK(corr == Vector<3>{0, 1, -1});
}