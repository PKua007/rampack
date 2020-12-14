//
// Created by Piotr Kubala on 13/12/2020.
//

#include <catch2/catch.hpp>

#include "matchers/PackingApproxPositionsCatchMatcher.h"

#include "core/Packing.h"
#include "core/Sphere.h"
#include "core/PeriodicBoundaryConditions.h"

TEST_CASE("Packing") {
    // Packing has linear scale of 5, so all coordinates in translate are multiplied by 3 (before scaling of course)
    Sphere sphere(0.25);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>(1);
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(sphere.clone());
    shapes.push_back(sphere.clone());
    shapes.push_back(sphere.clone());
    shapes[0]->translate({0.1, 0.1, 0.1}, *pbc);
    shapes[1]->translate({0.9, 0.1, 0.1}, *pbc);
    shapes[2]->translate({0.5, 0.5, 0.8}, *pbc);
    Packing packing(5, std::move(shapes), std::move(pbc));

    REQUIRE(packing.getLinearSize() == 5);

    SECTION("scaling upwards") {
        REQUIRE(packing.tryScaling(std::pow(1.1, 3)));
        CHECK(packing.getLinearSize() == Approx(5.5));
        CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.1, 0.1, 0.1}, {0.9, 0.1, 0.1}, {0.5, 0.5, 0.8}}, 1e-9));
    }

    SECTION("scaling downward without overlapping") {
        // For scale 0.5 => linear size = 5*0.5 = 2.5 spheres 0 and 1 are touching (through pbc).
        // So a little bit more should prevent any overlaps
        REQUIRE(packing.tryScaling(std::pow(0.501, 3)));
        CHECK(packing.getLinearSize() == Approx(2.505));
        CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.1, 0.1, 0.1}, {0.9, 0.1, 0.1}, {0.5, 0.5, 0.8}}, 1e-9));
    }

    SECTION("scaling downward with overlapping") {
        // Same as above, but a litte bit more gives an overlap
        REQUIRE_FALSE(packing.tryScaling(std::pow(0.499, 3)));
        CHECK(packing.getLinearSize() == Approx(5));
        CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.1, 0.1, 0.1}, {0.9, 0.1, 0.1}, {0.5, 0.5, 0.8}}, 1e-9));
    }

    SECTION("non-overlapping translation") {
        // For scale 5, translation {2, 2.5, -3.5} places particle 2 at {4.5, 5, 0.5}, while particle 1 is at
        // {4.5, 0.5, 0.5} - they touch through pbc on y coordinate. Do a little bit less prevents overlap
        REQUIRE(packing.tryTranslation(2, {2, 2.495, -3.5}));
        CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.1, 0.1, 0.1}, {0.9, 0.1, 0.1}, {0.9, 0.999, 0.1}}, 1e-9));
    }

    SECTION("overlapping translation") {
        // Same as above, but we do more instead of less
        REQUIRE_FALSE(packing.tryTranslation(2, {2, 2.505, -3.5}));
        CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.1, 0.1, 0.1}, {0.9, 0.1, 0.1}, {0.5, 0.5, 0.8}}, 1e-9));
    }
}