//
// Created by Codex on 12/07/2026.
//

#include <catch2/catch.hpp>

#include "core/external_fields/GravityField.h"
#include "mocks/MockShapeGeometry.h"
#include "utils/Exceptions.h"


TEST_CASE("GravityField: name") {
    const GravityField field(1, {0, 0, -1});

    CHECK(field.getName() == "gravity");
}

TEST_CASE("GravityField: validation") {
    SECTION("g has to be positive") {
        CHECK_THROWS_AS(GravityField(0, {0, 0, -1}), PreconditionException);
        CHECK_THROWS_AS(GravityField(-1, {0, 0, -1}), PreconditionException);
    }

    SECTION("direction has to have non-zero norm") {
        CHECK_THROWS_AS(GravityField(1, {0, 0, 0}), PreconditionException);
    }

    SECTION("box anchor has to be in unit interval") {
        CHECK_THROWS_AS(GravityField(1, {0, 0, -1}, std::nullopt, {-0.1, 0, 0}), PreconditionException);
        CHECK_THROWS_AS(GravityField(1, {0, 0, -1}, std::nullopt, {0, 1.1, 0}), PreconditionException);
    }

    SECTION("explicit point has to exist in shape geometry") {
        GravityField field(1, {0, 0, -1}, "missing");
        MockShapeGeometry geometry;

        CHECK_THROWS_AS(field.setupForShapeGeometry(geometry), PreconditionException);
    }
}

TEST_CASE("GravityField: point selection") {
    SECTION("auto point uses centre of mass if present") {
        GravityField field(1, {0, 0, -1});
        MockShapeGeometry geometry;
        geometry.addNamedPoints({{"cm", {0, 0, 2}}});

        field.setupForShapeGeometry(geometry);
        field.setupForBox(TriclinicBox(20));

        CHECK(field.calculateEnergy({0, 0, 1}, Matrix<3, 3>::identity()) == Approx(3));
    }

    SECTION("auto point falls back to geometric origin") {
        using trompeloeil::_;

        GravityField field(1, {0, 0, -1});
        MockShapeGeometry geometry;
        ALLOW_CALL(geometry, getGeometricOrigin(_)).RETURN(Vector<3>{0, 0, 4});

        field.setupForShapeGeometry(geometry);
        field.setupForBox(TriclinicBox(20));

        CHECK(field.calculateEnergy({0, 0, 1}, Matrix<3, 3>::identity()) == Approx(5));
    }

    SECTION("explicit point is used and rotates with shape") {
        GravityField field(2, {0, -1, 0}, "tip");
        MockShapeGeometry geometry;
        geometry.addNamedPoints({{"tip", {1, 0, 0}}});

        field.setupForShapeGeometry(geometry);
        field.setupForBox(TriclinicBox(4));

        auto rotation = Matrix<3, 3>::rotation({0, 0, 1}, M_PI / 2);
        CHECK(field.calculateEnergy({1, 1, 1}, rotation) == Approx(4));
    }
}

TEST_CASE("GravityField: box setup and energy") {
    SECTION("direction_hkl is interpreted in reciprocal box-normal coordinates") {
        GravityField field(1, {-1, 0, 0});
        MockShapeGeometry geometry;
        geometry.addNamedPoints({{"cm", {0, 0, 0}}});
        TriclinicBox box(Matrix<3, 3>{2, -1, 0,
                                      0, 3, 0,
                                      0, 0, 4});

        field.setupForShapeGeometry(geometry);
        field.setupForBox(box);

        Vector<3> directionUp{3 / std::sqrt(10.), 1 / std::sqrt(10.), 0};
        CHECK(field.calculateEnergy(directionUp, Matrix<3, 3>::identity()) == Approx(1));
    }

    SECTION("box anchor shifts zero potential point") {
        GravityField field(1, {0, 0, 1}, std::nullopt, {0, 0, 1});
        MockShapeGeometry geometry;
        geometry.addNamedPoints({{"cm", {0, 0, 0}}});

        field.setupForShapeGeometry(geometry);
        field.setupForBox(TriclinicBox(std::array<double, 3>{2, 3, 4}));

        CHECK(field.calculateEnergy({0, 0, 4}, Matrix<3, 3>::identity()) == Approx(0));
        CHECK(field.calculateEnergy({0, 0, 3}, Matrix<3, 3>::identity()) == Approx(1));
    }
}

TEST_CASE("GravityField: continuity") {
    const GravityField field(1, {0, 2, 1e-13});

    CHECK(field.getContinuityAlongBoxAxes() == std::array<bool, 3>{true, false, true});
}
