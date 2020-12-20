//
// Created by Piotr Kubala on 13/12/2020.
//

#include <catch2/catch.hpp>
#include <cmath>

#include "matchers/PackingApproxPositionsCatchMatcher.h"

#include "core/Packing.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/Interaction.h"

namespace {
    class SphereHardCoreInteraction : public Interaction {
    private:
        double radius;

    public:
        explicit SphereHardCoreInteraction(double radius) : radius(radius) { }

        [[nodiscard]] bool hasHardPart() const override { return true; }
        [[nodiscard]] bool hasSoftPart() const override { return false; }

        [[nodiscard]] double calculateEnergyBetween([[maybe_unused]] const Shape &shape1,
                                                    [[maybe_unused]] const Shape &shape2, [[maybe_unused]] double scale,
                                                    [[maybe_unused]] const BoundaryConditions &bc) const override
        {
            return 0;
        }

        [[nodiscard]] bool overlapBetween(const Shape &shape1, const Shape &shape2, double scale,
                                          const BoundaryConditions &bc) const override
        {
            return bc.getDistance2(shape1.getPosition(), shape2.getPosition()) < std::pow(2*this->radius/scale, 2);
        }
    };
}

TEST_CASE("Packing") {
    // Packing has linear scale of 5, so all coordinates in translate are multiplied by 3 (before scaling of course)
    double radius = 0.25;
    SphereHardCoreInteraction interaction(radius);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>(1);
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Shape>(Vector<3>{0.1, 0.1, 0.1}));
    shapes.push_back(std::make_unique<Shape>(Vector<3>{0.9, 0.1, 0.1}));
    shapes.push_back(std::make_unique<Shape>(Vector<3>{0.5, 0.5, 0.8}));
    Packing packing(5, std::move(shapes), std::move(pbc));

    REQUIRE(packing.getLinearSize() == 5);

    constexpr double inf = std::numeric_limits<double>::infinity();

    SECTION("scaling upwards") {
        REQUIRE(packing.tryScaling(std::pow(1.1, 3), interaction) == 0);
        CHECK(packing.getLinearSize() == Approx(5.5));
        CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.1, 0.1, 0.1}, {0.9, 0.1, 0.1}, {0.5, 0.5, 0.8}}, 1e-9));
    }

    SECTION("scaling downward without overlapping") {
        // For scale 0.5 => linear size = 5*0.5 = 2.5 spheres 0 and 1 are touching (through pbc).
        // So a little bit more should prevent any overlaps
        REQUIRE(packing.tryScaling(std::pow(0.501, 3), interaction) == 0);
        CHECK(packing.getLinearSize() == Approx(2.505));
        CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.1, 0.1, 0.1}, {0.9, 0.1, 0.1}, {0.5, 0.5, 0.8}}, 1e-9));
    }

    SECTION("scaling downward with overlapping") {
        // Same as above, but a litte bit more gives an overlap

        REQUIRE(packing.tryScaling(std::pow(0.499, 3), interaction) == inf);
        packing.revertScaling();
        CHECK(packing.getLinearSize() == Approx(5));
        CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.1, 0.1, 0.1}, {0.9, 0.1, 0.1}, {0.5, 0.5, 0.8}}, 1e-9));
    }

    SECTION("non-overlapping translation") {
        // For scale 5, translation {2, 2.5, -3.5} places particle 2 at {4.5, 5, 0.5}, while particle 1 is at
        // {4.5, 0.5, 0.5} - they touch through pbc on y coordinate. Do a little bit less prevents overlap
        REQUIRE(packing.tryTranslation(2, {2, 2.495, -3.5}, interaction) == 0);
        CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.1, 0.1, 0.1}, {0.9, 0.1, 0.1}, {0.9, 0.999, 0.1}}, 1e-9));
    }

    SECTION("overlapping translation") {
        // Same as above, but we do more instead of less
        REQUIRE(packing.tryTranslation(2, {2, 2.505, -3.5}, interaction) == inf);
        packing.revertTranslation();
        CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.1, 0.1, 0.1}, {0.9, 0.1, 0.1}, {0.5, 0.5, 0.8}}, 1e-9));
    }

    SECTION("packing fraction") {
        double sphereVolume = 4./3*M_PI*std::pow(radius, 3);
        CHECK(packing.getPackingFraction(sphereVolume) == Approx(0.001570796326794897));
    }

    SECTION("number density") {
        CHECK(packing.getNumberDensity() == Approx(0.024));
    }
}