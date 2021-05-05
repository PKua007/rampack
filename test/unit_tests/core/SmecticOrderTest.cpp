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

TEST_CASE("SmecticOrder") {
    SphereTraits traits;
    SmecticOrder smecticOrder({4, 4, 4});

    SECTION("meta") {
        CHECK(smecticOrder.getIntervalHeader() == std::vector<std::string>{"tau"});
        CHECK(smecticOrder.getNominalHeader() == std::vector<std::string>{"k_tau"});
        CHECK(smecticOrder.getName() == "smectic order");
    }

    SECTION("value") {
        // 30 x 30 x 30 packing, 3 layers on x: 0, 10, 20, y, z - random
        std::vector<Shape> shapes(27);
        std::fill(shapes.begin(), shapes.begin() + 9, Shape({0, 0, 0}));
        std::fill(shapes.begin() + 9, shapes.begin() + 18, Shape({10, 0, 0}));
        std::fill(shapes.begin() + 18, shapes.end(), Shape({20, 0, 0}));
        std::mt19937 mt(1234ul); // NOLINT(cert-msc51-cpp)
        std::uniform_real_distribution<double> unif(0, 30);
        auto fbc = std::make_unique<FreeBoundaryConditions>();
        for (auto &shape : shapes)
            shape.translate({0, unif(mt), unif(mt)}, *fbc);
        Packing packing({30, 30, 30}, shapes, std::move(fbc), traits.getInteraction());

        smecticOrder.calculate(packing, 1, 1, traits);
        auto intervalValues = smecticOrder.getIntervalValues();
        auto nominalValues = smecticOrder.getNominalValues();

        REQUIRE(intervalValues.size() == 1);
        CHECK(intervalValues[0] == Approx(0.001));
        REQUIRE(nominalValues == std::vector<std::string>{"3.0.0"});
    }
}