//
// Created by Piotr Kubala on 05/05/2021.
//

#include <catch2/catch.hpp>
#include <algorithm>
#include <random>
#include <memory>

#include "core/observables/SmecticOrder.h"
#include "core/FreeBoundaryConditions.h"
#include "core/shapes/SphereTraits.h"
#include "core/shapes/SpherocylinderTraits.h"
#include "core/observables/shape_functions/ShapeAxisCoordinate.h"


TEST_CASE("SmecticOrder: with vector dump") {
    SphereTraits traits;
    using A = std::array<std::size_t, 3>;
    // One ntauRanges without zeros and one with zeros (the one with zero was buggy)
    // Both include the actual best k vector with Miller indices 3.0.0
    auto nTauRanges = GENERATE(A{4, 4, 4}, A{4, 0, 0});
    SmecticOrder smecticOrder(nTauRanges, true);

    DYNAMIC_SECTION("tau ranges: {" << nTauRanges[0] << ", " << nTauRanges[1] << ", " << nTauRanges[2] << "}") {
        // 30 x 30 x 30 packing, 3 layers on x: 0, 10, 20, y, z - random
        std::vector<Shape> shapes(27);
        std::fill(shapes.begin(), shapes.begin() + 9, Shape({0, 0, 0}));
        std::fill(shapes.begin() + 9, shapes.begin() + 18, Shape({10, 0, 0}));
        std::fill(shapes.begin() + 18, shapes.end(), Shape({20, 0, 0}));
        std::mt19937 mt(1234ul); // NOLINT(cert-msc51-cpp)
        std::uniform_real_distribution<double> unif(0, 30);
        auto fbc = std::make_unique<FreeBoundaryConditions>();
        for (auto &shape: shapes)
            shape.translate({0, unif(mt), unif(mt)}, *fbc);
        Packing packing({30, 30, 30}, shapes, std::move(fbc), traits.getInteraction());

        smecticOrder.calculate(packing, 1, 1, traits);
        auto intervalValues = smecticOrder.getIntervalValues();
        auto nominalValues = smecticOrder.getNominalValues();

        REQUIRE(intervalValues.size() == 4);
        CHECK(intervalValues[0] == Approx(0.001));
        CHECK(intervalValues[1] == Approx(2 * M_PI / 10));
        CHECK(intervalValues[2] == Approx(0));
        CHECK(intervalValues[3] == Approx(0));
        CHECK(nominalValues == std::vector<std::string>{"3.0.0"});
    }
}

TEST_CASE("SmecticOrder: non-standard focal point") {
    SpherocylinderTraits traits(1, 0.5);
    SmecticOrder smecticOrder({3, 3, 3}, false, "end");

    // Two layers along z, where second (positive) spherocylinders' caps perfectly meet on z coordinate. x and y
    // coordinates have worse order
    auto rotated = Matrix<3, 3>::rotation(M_PI, 0, 0);
    auto notRotated = Matrix<3, 3>::identity();
    std::vector<Shape> shapes = {{{2, 2, 1.5}, notRotated}, {{2, 5, 2.5}, rotated},
                                 {{5, 5, 1.5}, notRotated}, {{5, 2, 2.5}, rotated},
                                 {{2, 2, 5.5}, notRotated}, {{2, 5, 6.5}, rotated},
                                 {{5, 5, 5.5}, notRotated}, {{5, 2, 6.5}, rotated}};
    auto fbc = std::make_unique<FreeBoundaryConditions>();
    Packing packing({8, 8, 8}, shapes, std::move(fbc), traits.getInteraction());

    smecticOrder.calculate(packing, 1, 1, traits);
    auto intervalValues = smecticOrder.getIntervalValues();
    auto nominalValues = smecticOrder.getNominalValues();

    CHECK(intervalValues[0] == Approx(8/(8.0*8*8)));
    CHECK(nominalValues == std::vector<std::string>{"0.0.2"});
}

TEST_CASE("SmecticOrder: with function") {
    SpherocylinderTraits traits(1, 0.5);
    auto axis = ShapeGeometry::Axis::PRIMARY;
    auto shapeAxisCoordinate = std::make_shared<ShapeAxisCoordinate>(axis, 2);
    SmecticOrder smecticOrder({0, 0, 3}, false, "o", shapeAxisCoordinate);

    // Two shapes - one rotated, one not; it is a toy model of two layers, one with polarization "up", one with
    // polarization "down". If we used a normal smectic order, it would be 1 for hkl = 002. But we feed is with
    // polarization z coordinate, so it is 1 for hkl = 001.
    auto rotated = Matrix<3, 3>::rotation(M_PI, 0, 0);
    auto notRotated = Matrix<3, 3>::identity();
    std::vector<Shape> shapes = {{{2, 2, 1}, notRotated}, {{2, 2, 3}, rotated}};
    auto fbc = std::make_unique<FreeBoundaryConditions>();
    Packing packing({4, 4, 4}, shapes, std::move(fbc), traits.getInteraction());

    smecticOrder.calculate(packing, 1, 1, traits);
    auto intervalValues = smecticOrder.getIntervalValues();
    auto nominalValues = smecticOrder.getNominalValues();

    CHECK(intervalValues[0] == Approx(2./(4*4*4)));
    CHECK(nominalValues == std::vector<std::string>{"0.0.1"});
}

TEST_CASE("SmecticOrder: meta") {
    SECTION("const + no vector dump") {
        SmecticOrder smecticOrder({3, 3, 3}, false);
        CHECK(smecticOrder.getName() == "smectic order");
        CHECK(smecticOrder.getIntervalHeader() == std::vector<std::string>{"tau"});
        CHECK(smecticOrder.getNominalHeader() == std::vector<std::string>{"tau_hkl"});
    }

    SECTION("const + vector dump") {
        SmecticOrder smecticOrder({3, 3, 3}, true);
        CHECK(smecticOrder.getName() == "smectic order");
        CHECK(smecticOrder.getIntervalHeader() == std::vector<std::string>{"tau", "tau_k_x", "tau_k_y", "tau_k_z"});
        CHECK(smecticOrder.getNominalHeader() == std::vector<std::string>{"tau_hkl"});
    }

    auto primaryAxisX = std::make_shared<ShapeAxisCoordinate>(ShapeGeometry::Axis::PRIMARY, 0);

    SECTION("pa_x + no vector dump") {
        SmecticOrder smecticOrder({3, 3, 3}, false, "o", primaryAxisX);
        CHECK(smecticOrder.getName() == "pa_x smectic order");
        CHECK(smecticOrder.getIntervalHeader() == std::vector<std::string>{"tau_pa_x"});
        CHECK(smecticOrder.getNominalHeader() == std::vector<std::string>{"tau_pa_x_hkl"});
    }

    SECTION("pa_x + vector dump") {
        SmecticOrder smecticOrder({3, 3, 3}, true, "o", primaryAxisX);
        CHECK(smecticOrder.getName() == "pa_x smectic order");
        CHECK(smecticOrder.getIntervalHeader()
              == std::vector<std::string>{"tau_pa_x", "tau_pa_x_k_x", "tau_pa_x_k_y", "tau_pa_x_k_z"});
        CHECK(smecticOrder.getNominalHeader() == std::vector<std::string>{"tau_pa_x_hkl"});
    }
}