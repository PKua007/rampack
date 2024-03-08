//
// Created by Piotr Kubala on 13/12/2020.
//

#include <catch2/catch.hpp>

#include "matchers/MatrixApproxMatcher.h"

#include "core/lattice/legacy/OrthorhombicArrangingModel.h"


using namespace legacy;

using Polarization = OrthorhombicArrangingModel::Polarization;
using Axis = OrthorhombicArrangingModel::Axis;
using Clinicity = OrthorhombicArrangingModel::Clinicity;

TEST_CASE("OrthorhombicArrangingModel: polar") {
    OrthorhombicArrangingModel model;

    SECTION("not fully filled lattice") {
        auto shapes = model.arrange(7, {2, 2, 2});

        REQUIRE(shapes.size() == 7);
        CHECK(shapes[0].getPosition() == Vector<3>{0.5, 0.5, 0.5});
        CHECK(shapes[1].getPosition() == Vector<3>{0.5, 0.5, 1.5});
        CHECK(shapes[2].getPosition() == Vector<3>{0.5, 1.5, 0.5});
        CHECK(shapes[3].getPosition() == Vector<3>{0.5, 1.5, 1.5});
        CHECK(shapes[4].getPosition() == Vector<3>{1.5, 0.5, 0.5});
        CHECK(shapes[5].getPosition() == Vector<3>{1.5, 0.5, 1.5});
        CHECK(shapes[6].getPosition() == Vector<3>{1.5, 1.5, 0.5});
    }

    SECTION("fully filled lattice") {
        auto shapes = model.arrange(8, {2, 2, 2});

        REQUIRE(shapes.size() == 8);
        CHECK(shapes[0].getPosition() == Vector<3>{0.5, 0.5, 0.5});
        CHECK(shapes[1].getPosition() == Vector<3>{0.5, 0.5, 1.5});
        CHECK(shapes[2].getPosition() == Vector<3>{0.5, 1.5, 0.5});
        CHECK(shapes[3].getPosition() == Vector<3>{0.5, 1.5, 1.5});
        CHECK(shapes[4].getPosition() == Vector<3>{1.5, 0.5, 0.5});
        CHECK(shapes[5].getPosition() == Vector<3>{1.5, 0.5, 1.5});
        CHECK(shapes[6].getPosition() == Vector<3>{1.5, 1.5, 0.5});
        CHECK(shapes[7].getPosition() == Vector<3>{1.5, 1.5, 1.5});
    }
}

TEST_CASE("OrthorhombicArrangingModel: antipolar") {
    OrthorhombicArrangingModel model(Polarization::ANTIFERRO, Axis::Y);

    auto shapes = model.arrange(8, {2, 2, 2});

    Matrix<3, 3> identity = Matrix<3, 3>::identity();
    Matrix<3, 3> rotated = Matrix<3, 3>::rotation(0, M_PI, 0);
    REQUIRE(shapes.size() == 8);
    CHECK(shapes[0].getPosition() == Vector<3>{0.5, 0.5, 0.5});
    CHECK_THAT(shapes[0].getOrientation(), IsApproxEqual(identity, 1e-10));
    CHECK(shapes[1].getPosition() == Vector<3>{0.5, 0.5, 1.5});
    CHECK_THAT(shapes[1].getOrientation(), IsApproxEqual(identity, 1e-10));
    CHECK(shapes[2].getPosition() == Vector<3>{0.5, 1.5, 0.5});
    CHECK_THAT(shapes[2].getOrientation(), IsApproxEqual(rotated, 1e-10));
    CHECK(shapes[3].getPosition() == Vector<3>{0.5, 1.5, 1.5});
    CHECK_THAT(shapes[3].getOrientation(), IsApproxEqual(rotated, 1e-10));
    CHECK(shapes[4].getPosition() == Vector<3>{1.5, 0.5, 0.5});
    CHECK_THAT(shapes[4].getOrientation(), IsApproxEqual(identity, 1e-10));
    CHECK(shapes[5].getPosition() == Vector<3>{1.5, 0.5, 1.5});
    CHECK_THAT(shapes[5].getOrientation(), IsApproxEqual(identity, 1e-10));
    CHECK(shapes[6].getPosition() == Vector<3>{1.5, 1.5, 0.5});
    CHECK_THAT(shapes[6].getOrientation(), IsApproxEqual(rotated, 1e-10));
    CHECK(shapes[7].getPosition() == Vector<3>{1.5, 1.5, 1.5});
    CHECK_THAT(shapes[7].getOrientation(), IsApproxEqual(rotated, 1e-10));
}

TEST_CASE("OrthorhombicArrangingModel: tilted") {
    SECTION("synclinic ferro") {
        OrthorhombicArrangingModel model(Polarization::FERRO, Axis::Y, Clinicity::SYNCLINIC, Axis::X, 0.5);

        auto shapes = model.arrange(8, {2, 2, 2});

        Matrix<3, 3> tilted = Matrix<3, 3>::rotation(0.5, 0, 0);
        REQUIRE(shapes.size() == 8);
        CHECK(shapes[0].getPosition() == Vector<3>{0.5, 0.5, 0.5});
        CHECK_THAT(shapes[0].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[1].getPosition() == Vector<3>{0.5, 0.5, 1.5});
        CHECK_THAT(shapes[1].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[2].getPosition() == Vector<3>{0.5, 1.5, 0.5});
        CHECK_THAT(shapes[2].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[3].getPosition() == Vector<3>{0.5, 1.5, 1.5});
        CHECK_THAT(shapes[3].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[4].getPosition() == Vector<3>{1.5, 0.5, 0.5});
        CHECK_THAT(shapes[4].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[5].getPosition() == Vector<3>{1.5, 0.5, 1.5});
        CHECK_THAT(shapes[5].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[6].getPosition() == Vector<3>{1.5, 1.5, 0.5});
        CHECK_THAT(shapes[6].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[7].getPosition() == Vector<3>{1.5, 1.5, 1.5});
        CHECK_THAT(shapes[7].getOrientation(), IsApproxEqual(tilted, 1e-10));
    }

    SECTION("anticlinic ferro") {
        OrthorhombicArrangingModel model(Polarization::FERRO, Axis::Y, Clinicity::ANTICLINIC, Axis::X, 0.5);

        auto shapes = model.arrange(8, {2, 2, 2});

        Matrix<3, 3> tilted = Matrix<3, 3>::rotation(0.5, 0, 0);
        Matrix<3, 3> antiTilted = Matrix<3, 3>::rotation(-0.5, 0, 0);
        REQUIRE(shapes.size() == 8);
        CHECK(shapes[0].getPosition() == Vector<3>{0.5, 0.5, 0.5});
        CHECK_THAT(shapes[0].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[1].getPosition() == Vector<3>{0.5, 0.5, 1.5});
        CHECK_THAT(shapes[1].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[2].getPosition() == Vector<3>{0.5, 1.5, 0.5});
        CHECK_THAT(shapes[2].getOrientation(), IsApproxEqual(antiTilted, 1e-10));
        CHECK(shapes[3].getPosition() == Vector<3>{0.5, 1.5, 1.5});
        CHECK_THAT(shapes[3].getOrientation(), IsApproxEqual(antiTilted, 1e-10));
        CHECK(shapes[4].getPosition() == Vector<3>{1.5, 0.5, 0.5});
        CHECK_THAT(shapes[4].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[5].getPosition() == Vector<3>{1.5, 0.5, 1.5});
        CHECK_THAT(shapes[5].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[6].getPosition() == Vector<3>{1.5, 1.5, 0.5});
        CHECK_THAT(shapes[6].getOrientation(), IsApproxEqual(antiTilted, 1e-10));
        CHECK(shapes[7].getPosition() == Vector<3>{1.5, 1.5, 1.5});
        CHECK_THAT(shapes[7].getOrientation(), IsApproxEqual(antiTilted, 1e-10));
    }

    SECTION("anticlinic antiferro") {
        OrthorhombicArrangingModel model(Polarization::ANTIFERRO, Axis::Y, Clinicity::ANTICLINIC, Axis::X, 0.5);

        auto shapes = model.arrange(8, {2, 2, 2});

        Matrix<3, 3> tilted = Matrix<3, 3>::rotation(0.5, 0, 0);
        Matrix<3, 3> antiTilted = Matrix<3, 3>::rotation(-0.5, 0, 0) * Matrix<3, 3>::rotation(0, M_PI, 0);
        REQUIRE(shapes.size() == 8);
        CHECK(shapes[0].getPosition() == Vector<3>{0.5, 0.5, 0.5});
        CHECK_THAT(shapes[0].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[1].getPosition() == Vector<3>{0.5, 0.5, 1.5});
        CHECK_THAT(shapes[1].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[2].getPosition() == Vector<3>{0.5, 1.5, 0.5});
        CHECK_THAT(shapes[2].getOrientation(), IsApproxEqual(antiTilted, 1e-10));
        CHECK(shapes[3].getPosition() == Vector<3>{0.5, 1.5, 1.5});
        CHECK_THAT(shapes[3].getOrientation(), IsApproxEqual(antiTilted, 1e-10));
        CHECK(shapes[4].getPosition() == Vector<3>{1.5, 0.5, 0.5});
        CHECK_THAT(shapes[4].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[5].getPosition() == Vector<3>{1.5, 0.5, 1.5});
        CHECK_THAT(shapes[5].getOrientation(), IsApproxEqual(tilted, 1e-10));
        CHECK(shapes[6].getPosition() == Vector<3>{1.5, 1.5, 0.5});
        CHECK_THAT(shapes[6].getOrientation(), IsApproxEqual(antiTilted, 1e-10));
        CHECK(shapes[7].getPosition() == Vector<3>{1.5, 1.5, 1.5});
        CHECK_THAT(shapes[7].getOrientation(), IsApproxEqual(antiTilted, 1e-10));
    }
}