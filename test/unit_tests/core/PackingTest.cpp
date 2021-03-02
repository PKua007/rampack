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

        [[nodiscard]] bool overlapBetween(const Vector<3> &pos1,
                                          [[maybe_unused]] const Matrix<3, 3> &orientaton1,
                                          [[maybe_unused]] std::size_t idx1,
                                          const Vector<3> &pos2,
                                          [[maybe_unused]] const Matrix<3, 3> &orientaton2,
                                          [[maybe_unused]] std::size_t idx2,
                                          const BoundaryConditions &bc) const override
        {
            return bc.getDistance2(pos1, pos2) < std::pow(2*this->radius, 2);
        }

        [[nodiscard]] double getRangeRadius() const override { return 2*this->radius; }
    };

    class SphereDistanceInteraction : public Interaction {
    public:
        [[nodiscard]] bool hasHardPart() const override { return false; }
        [[nodiscard]] bool hasSoftPart() const override { return true; }

        [[nodiscard]] double calculateEnergyBetween(const Vector<3> &pos1,
                                                    [[maybe_unused]] const Matrix<3, 3> &orientaton1,
                                                    [[maybe_unused]] std::size_t idx1,
                                                    const Vector<3> &pos2,
                                                    [[maybe_unused]] const Matrix<3, 3> &orientaton2,
                                                    [[maybe_unused]] std::size_t idx2,
                                                    const BoundaryConditions &bc) const override
        {
            return std::sqrt(bc.getDistance2(pos1, pos2));
        }
    };
}

TEST_CASE("Packing") {
    // Packing has linear scale of 5, so all coordinates in translate are multiplied by 3 (before scaling of course)
    double radius = 0.25;
    SphereHardCoreInteraction hardCore(radius);
    SphereDistanceInteraction distanceInteraction{};
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes;
    shapes.emplace_back(Vector<3>{0.5, 0.5, 0.5});
    shapes.emplace_back(Vector<3>{4.5, 0.5, 0.5});
    shapes.emplace_back(Vector<3>{2.5, 2.5, 4.0});
    Packing packing({5, 5, 5}, std::move(shapes), std::move(pbc), hardCore);

    REQUIRE(packing.getDimensions() == std::array<double, 3>{5, 5, 5});

    constexpr double inf = std::numeric_limits<double>::infinity();

    SECTION ("scaling") {
        SECTION("hard core upwards") {
            CHECK(packing.tryScaling(std::pow(1.1, 3), hardCore) == 0);
            CHECK(packing.getDimensions()[0] == Approx(5.5));
            CHECK(packing.getDimensions()[1] == Approx(5.5));
            CHECK(packing.getDimensions()[2] == Approx(5.5));
            CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.55, 0.55, 0.55}, {4.95, 0.55, 0.55},
                                                                 {2.75, 2.75, 4.4}}, 1e-9));
        }

        SECTION("hard core downward without overlapping") {
            // For scale 0.5 => linear size = 5*0.5 = 2.5 spheres 0 and 1 are touching (through pbc).
            // So a little bit more should prevent any overlaps
            CHECK(packing.tryScaling(std::pow(0.51, 3), hardCore) == 0);
            CHECK(packing.getDimensions()[0] == Approx(2.55));
            CHECK(packing.getDimensions()[1] == Approx(2.55));
            CHECK(packing.getDimensions()[2] == Approx(2.55));
            CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.255, 0.255, 0.255}, {2.295, 0.255, 0.255},
                                                                 {1.275, 1.275, 2.04}}, 1e-9));
        }

        SECTION("hard core downward with overlapping") {
            // Same as above, but a little bit more gives an overlap
            REQUIRE(packing.tryScaling(std::pow(0.49, 3), hardCore) == inf);
            CHECK(packing.getDimensions()[0] == Approx(2.45));
            CHECK(packing.getDimensions()[1] == Approx(2.45));
            CHECK(packing.getDimensions()[2] == Approx(2.45));

            SECTION("reverting the move") {
                packing.revertScaling();
                CHECK(packing.getDimensions()[0] == Approx(5));
                CHECK(packing.getDimensions()[1] == Approx(5));
                CHECK(packing.getDimensions()[2] == Approx(5));
                CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.5, 0.5, 0.5}, {4.5, 0.5, 0.5}, {2.5, 2.5, 4.0}}, 1e-9));
            }
        }

        SECTION("distance interaction") {
            packing.setupForInteraction(distanceInteraction);
            // 1 <-> 2: d = 2/10
            // 1 <-> 3: d = sqrt(0.4^2 + 0.4^2 + 0.3^2) = sqrt(41)/10
            // 2 <-> 3: d = sqrt(0.4^2 + 0.4^2 + 0.3^2) = sqrt(41)/10
            // We scale downward from 5 to 2.5
            double scale1dE = (2 + 2*std::sqrt(41))/10;
            double dE = scale1dE * (2.5 - 5);
            CHECK(packing.tryScaling(0.125, distanceInteraction) == Approx(dE));
        }
    }

    SECTION("translating") {
        SECTION("non-overlapping") {
            // For scale 5, translation {2, 2.5, -3.5} places particle 2 at {4.5, 5, 0.5}, while particle 1 is at
            // {4.5, 0.5, 0.5} - they touch through pbc on y coordinate. Do a little bit less prevents overlap
            CHECK(packing.tryTranslation(2, {2, 2.45, -3.5}, hardCore) == 0);
            CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.5, 0.5, 0.5}, {4.5, 0.5, 0.5}, {2.5, 2.5, 4.0}}, 1e-9));
            SECTION("accepting the move") {
                packing.acceptTranslation();
                CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.5, 0.5, 0.5}, {4.5, 0.5, 0.5}, {4.5, 4.95, 0.5}}, 1e-9));
            }
        }

        SECTION("overlapping") {
            // Same as above, but we do more instead of less
            CHECK(packing.tryTranslation(2, {2, 2.55, -3.5}, hardCore) == inf);
            CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.5, 0.5, 0.5}, {4.5, 0.5, 0.5}, {2.5, 2.5, 4.0}}, 1e-9));
        }

        SECTION("distance interaction") {
            packing.setupForInteraction(distanceInteraction);
            // first particle moved to {0.1, 0.1, 0.8}, so after (for before, look at scaling SECTION)
            // 1 <-> 2: d = sqrt(0.2^2 + 0.3^2) = sqrt(13)/10
            // 1 <-> 3: d = sqrt(0.4^2 + 0.4^2) = sqrt(32)/10
            // 2 <-> 3: d = sqrt(0.4^2 + 0.4^2 + 0.3^2) = sqrt(41)/10
            double scale1E0 = (2 + 2*std::sqrt(41))/10;
            double scale1E1 = (std::sqrt(13) + std::sqrt(32) + std::sqrt(41))/10;
            double dE = 5 * (scale1E1 - scale1E0);
            CHECK(packing.tryTranslation(0, {0, 0, 3.5}, distanceInteraction) == Approx(dE));
        }
    }

    SECTION("packing fraction") {
        double sphereVolume = 4./3*M_PI*std::pow(radius, 3);
        CHECK(packing.getPackingFraction(sphereVolume) == Approx(0.001570796326794897));
    }

    SECTION("number density") {
        CHECK(packing.getNumberDensity() == Approx(0.024));
    }

    SECTION("interaction energy") {
        packing.setupForInteraction(distanceInteraction);

        SECTION("total energy") {
            double scale1E = (2 + 2 * std::sqrt(41)) / 10;
            double E = scale1E * 5;
            CHECK(packing.getTotalEnergy(distanceInteraction) == Approx(E));
        }

        SECTION("energy fluctuations") {
            double scale1Efluct = (std::sqrt(41) - 2) / (20 * sqrt(3));
            double Efluct = scale1Efluct * 5;
            CHECK(packing.getParticleEnergyFluctuations(distanceInteraction) == Approx(Efluct));
        }

        // A method temporarily retired

        /*SECTION("particle energy") {
            CHECK(packing.calculateParticleEnergy(0, distanceInteraction) == Approx((2 + std::sqrt(41)) / 10 * 5));
            CHECK(packing.calculateParticleEnergy(1, distanceInteraction) == Approx((2 + std::sqrt(41)) / 10 * 5));
            CHECK(packing.calculateParticleEnergy(2, distanceInteraction) == Approx(2 * std::sqrt(41) / 10 * 5));
        }*/
    }
}