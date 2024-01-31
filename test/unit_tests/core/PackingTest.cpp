//
// Created by Piotr Kubala on 13/12/2020.
//

#include <catch2/catch.hpp>
#include <cmath>
#include <sstream>

#include "matchers/PackingApproxPositionsCatchMatcher.h"
#include "matchers/VectorApproxMatcher.h"

#include "mocks/MockShapeGeometry.h"

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
        [[nodiscard]] bool hasWallPart() const override { return true; }
        // Claim it is not convex to force full overlap check on upscaling moves
        [[nodiscard]] bool isConvex() const override { return false; }

        [[nodiscard]] bool overlapBetween(const Vector<3> &pos1,
                                          [[maybe_unused]] const Matrix<3, 3> &orientaton1,
                                          [[maybe_unused]] const std::byte *data1,
                                          [[maybe_unused]] std::size_t idx1,
                                          const Vector<3> &pos2,
                                          [[maybe_unused]] const Matrix<3, 3> &orientaton2,
                                          [[maybe_unused]] const std::byte *data2,
                                          [[maybe_unused]] std::size_t idx2,
                                          const BoundaryConditions &bc) const override
        {
            return bc.getDistance2(pos1, pos2) < std::pow(2*this->radius, 2);
        }

        [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, [[maybe_unused]] const Matrix<3, 3> &orientation,
                                           [[maybe_unused]] const std::byte *data, [[maybe_unused]] std::size_t idx,
                                           const Vector<3> &wallOrigin, const Vector<3> &wallVector) const override
        {
            double dotProduct = wallVector * (pos - wallOrigin);
            return dotProduct < this->radius;
        }

        [[nodiscard]] double getRangeRadius() const override { return 2*this->radius; }
    };

    class PolydisperseSphereHardCoreInteraction : public Interaction {
    public:
        using Radius = double;

    private:
        Radius maxRadius{};

    public:
        explicit PolydisperseSphereHardCoreInteraction(Radius maxRadius) : maxRadius{maxRadius} { }

        [[nodiscard]] bool hasHardPart() const override { return true; }
        [[nodiscard]] bool hasSoftPart() const override { return false; }
        [[nodiscard]] bool hasWallPart() const override { return true; }
        // Claim it is not convex to force full overlap check on upscaling moves
        [[nodiscard]] bool isConvex() const override { return false; }
        [[nodiscard]] std::size_t getShapeDataSize() const override { return sizeof(Radius); }

        [[nodiscard]] bool overlapBetween(const Vector<3> &pos1,
                                          [[maybe_unused]] const Matrix<3, 3> &orientaton1,
                                          const std::byte *data1,
                                          [[maybe_unused]] std::size_t idx1,
                                          const Vector<3> &pos2,
                                          [[maybe_unused]] const Matrix<3, 3> &orientaton2,
                                          const std::byte *data2,
                                          [[maybe_unused]] std::size_t idx2,
                                          const BoundaryConditions &bc) const override
        {
            Radius radius1 = ShapeData::as<Radius>(data1);
            Radius radius2 = ShapeData::as<Radius>(data2);
            return bc.getDistance2(pos1, pos2) < std::pow(radius1 + radius2, 2);
        }

        [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, [[maybe_unused]] const Matrix<3, 3> &orientation,
                                           [[maybe_unused]] const std::byte *data, [[maybe_unused]] std::size_t idx,
                                           const Vector<3> &wallOrigin, const Vector<3> &wallVector) const override
        {
            Radius radius = ShapeData::as<Radius>(data);
            double dotProduct = wallVector * (pos - wallOrigin);
            return dotProduct < radius;
        }

        [[nodiscard]] double getRangeRadius() const override { return 2*this->maxRadius; }
    };

    class SphereDistanceInteraction : public Interaction {
    public:
        [[nodiscard]] bool hasHardPart() const override { return false; }
        [[nodiscard]] bool hasSoftPart() const override { return true; }
        [[nodiscard]] bool hasWallPart() const override { return false; }
        [[nodiscard]] bool isConvex() const override { return false; }

        [[nodiscard]] double calculateEnergyBetween(const Vector<3> &pos1,
                                                    [[maybe_unused]] const Matrix<3, 3> &orientaton1,
                                                    [[maybe_unused]] const std::byte *data1,
                                                    [[maybe_unused]] std::size_t idx1,
                                                    const Vector<3> &pos2,
                                                    [[maybe_unused]] const Matrix<3, 3> &orientaton2,
                                                    [[maybe_unused]] const std::byte *data2,
                                                    [[maybe_unused]] std::size_t idx2,
                                                    const BoundaryConditions &bc) const override
        {
            return std::sqrt(bc.getDistance2(pos1, pos2));
        }
    };

    class PolydisperseElectrostaticInteraction : public Interaction {
    public:
        using Charge = double;

        [[nodiscard]] bool hasHardPart() const override { return false; }
        [[nodiscard]] bool hasSoftPart() const override { return true; }
        [[nodiscard]] bool hasWallPart() const override { return false; }
        [[nodiscard]] bool isConvex() const override { return false; }
        [[nodiscard]] std::size_t getShapeDataSize() const override { return sizeof(Charge); }

        [[nodiscard]] double calculateEnergyBetween(const Vector<3> &pos1,
                                                    [[maybe_unused]] const Matrix<3, 3> &orientaton1,
                                                    const std::byte *data1,
                                                    [[maybe_unused]] std::size_t idx1,
                                                    const Vector<3> &pos2,
                                                    [[maybe_unused]] const Matrix<3, 3> &orientaton2,
                                                    const std::byte *data2,
                                                    [[maybe_unused]] std::size_t idx2,
                                                    const BoundaryConditions &bc) const override
        {
            Charge q1 = ShapeData::as<Charge>(data1);
            Charge q2 = ShapeData::as<Charge>(data2);
            double r = std::sqrt(bc.getDistance2(pos1, pos2));

            return -q1*q2/r;
        }
    };

    class DimerDistanceInteraction : public SphereDistanceInteraction {
    public:
        [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const override { return {{0, 0, 0}, {1, 0, 0}}; }
    };

    class DimerHardCoreInteraction : public SphereHardCoreInteraction {
    public:
        explicit DimerHardCoreInteraction(double radius) : SphereHardCoreInteraction(radius) { }

        [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const override { return {{0, 0, 0}, {1, 0, 0}}; }
    };

    class PolysphereGeometry : public ShapeGeometry {
    public:
        [[nodiscard]] double getVolume([[maybe_unused]] const Shape &shape) const override {
            auto radius = shape.getData().as<double>();
            return 4./3*M_PI*std::pow(radius, 3);
        }
    };
}


TEST_CASE("Packing: hard single interaction center") {
    using Radius = PolydisperseSphereHardCoreInteraction::Radius;
    Radius maxRadius = 0.3;
    PolydisperseSphereHardCoreInteraction hardCore(maxRadius);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes;
    auto noRot = Matrix<3, 3>::identity();
    shapes.emplace_back(Vector<3>{0.5, 0.5, 0.5}, noRot, Radius{0.3});
    shapes.emplace_back(Vector<3>{4.5, 0.5, 0.5}, noRot, Radius{0.2});
    shapes.emplace_back(Vector<3>{2.5, 2.5, 4.0}, noRot, Radius{0.3});
    Packing packing({5, 5, 5}, std::move(shapes), std::move(pbc), hardCore);

    REQUIRE(packing.getBox().getHeights() == std::array<double, 3>{5, 5, 5});

    constexpr double inf = std::numeric_limits<double>::infinity();

    SECTION ("scaling") {
        SECTION("hard core upwards") {
            CHECK(packing.tryScaling(1.1, hardCore) == 0);
            CHECK(packing.getBox().getHeights()[0] == Approx(5.5));
            CHECK(packing.getBox().getHeights()[1] == Approx(5.5));
            CHECK(packing.getBox().getHeights()[2] == Approx(5.5));
            CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.55, 0.55, 0.55}, {4.95, 0.55, 0.55},
                                                                 {2.75, 2.75, 4.4}}, 1e-9));
        }

        SECTION("hard core downward without overlapping") {
            // For scale 0.5 => linear size = 5*0.5 = 2.5 spheres 0 and 1 are touching (through pbc).
            // So a little bit more should prevent any overlaps
            CHECK(packing.tryScaling(0.51, hardCore) == 0);
            CHECK(packing.getBox().getHeights()[0] == Approx(2.55));
            CHECK(packing.getBox().getHeights()[1] == Approx(2.55));
            CHECK(packing.getBox().getHeights()[2] == Approx(2.55));
            CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.255, 0.255, 0.255}, {2.295, 0.255, 0.255},
                                                                 {1.275, 1.275, 2.04}}, 1e-9));
        }

        SECTION("hard core downward with overlapping") {
            // Same as above, but a little bit more gives an overlap
            REQUIRE(packing.tryScaling(0.49, hardCore) == inf);
            CHECK(packing.getBox().getHeights()[0] == Approx(2.45));
            CHECK(packing.getBox().getHeights()[1] == Approx(2.45));
            CHECK(packing.getBox().getHeights()[2] == Approx(2.45));

            SECTION("reverting the move") {
                packing.revertScaling();
                CHECK(packing.getBox().getHeights()[0] == Approx(5));
                CHECK(packing.getBox().getHeights()[1] == Approx(5));
                CHECK(packing.getBox().getHeights()[2] == Approx(5));
                CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.5, 0.5, 0.5}, {4.5, 0.5, 0.5}, {2.5, 2.5, 4.0}}, 1e-9));
            }
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
    }

    SECTION("packing fraction") {
        PolysphereGeometry geometry;

        double spheresVolume = 4./3*M_PI*(0.2*0.2*0.2 + 0.3*0.3*0.3 + 0.3*0.3*0.3);
        double boxVolume = 5*5*5;
        CHECK(packing.getPackingFraction(geometry) == Approx(spheresVolume/boxVolume));
    }

    SECTION("number density") {
        CHECK(packing.getNumberDensity() == Approx(0.024));
    }
}

TEST_CASE("Packing: soft single interaction center") {
    using Charge = PolydisperseElectrostaticInteraction::Charge;
    PolydisperseElectrostaticInteraction electrostaticInteraction;
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes;
    auto noRot = Matrix<3, 3>::identity();
    shapes.emplace_back(Vector<3>{0.5, 0.5, 0.5}, noRot, Charge{1});
    shapes.emplace_back(Vector<3>{4.5, 0.5, 0.5}, noRot, Charge{2});
    shapes.emplace_back(Vector<3>{2.5, 2.5, 4.0}, noRot, Charge{3});
    Packing packing({5, 5, 5}, std::move(shapes), std::move(pbc), electrostaticInteraction);

    SECTION("scaling") {
        // 1 <-> 2: d = 2/10
        // 1 <-> 3: d = sqrt(0.4^2 + 0.4^2 + 0.3^2) = sqrt(41)/10
        // 2 <-> 3: d = sqrt(0.4^2 + 0.4^2 + 0.3^2) = sqrt(41)/10
        // We scale downward from 5 to 2.5
        double scale1E = -(1.*2)/0.2 + -(1.*3)/(std::sqrt(41)/10) + -(2.*3)/(std::sqrt(41)/10);
        double scale5E = scale1E * (1./5);          // 1/r proportionality
        double scale2p5E = scale1E * (1./2.5);      //
        double dE = scale2p5E - scale5E;
        CHECK(packing.tryScaling(0.5, electrostaticInteraction) == Approx(dE));
    }

    SECTION("translating") {
        // first particle moved to {0.1, 0.1, 0.8}, so after the move:
        // 1 <-> 2: d = sqrt(0.2^2 + 0.3^2) = sqrt(13)/10
        // 1 <-> 3: d = sqrt(0.4^2 + 0.4^2) = sqrt(32)/10
        // 2 <-> 3: d = sqrt(0.4^2 + 0.4^2 + 0.3^2) = sqrt(41)/10
        // for before the move, look at scaling SECTION
        double E0 = -(1.*2)/1 + -(1.*3)/(std::sqrt(41)/2) + -(2.*3)/(std::sqrt(41)/2);
        double E1 = -(1.*2)/(std::sqrt(13)/2) + -(1.*3)/(std::sqrt(32)/2) + -(2.*3)/(std::sqrt(41)/2);
        double dE = E1 - E0;
        CHECK(packing.tryTranslation(0, {0, 0, 3.5}, electrostaticInteraction) == Approx(dE));

        SECTION("correct energy before and after accepted move") {
            CHECK(packing.getTotalEnergy(electrostaticInteraction) == Approx(E0));
            packing.acceptTranslation();
            CHECK(packing.getTotalEnergy(electrostaticInteraction) == Approx(E1));
        }
    }

    SECTION("energy fluctuations") {
        double E12 = -(1.*2)/1;
        double E13 = -(1.*3)/(std::sqrt(41)/2);
        double E23 = -(2.*3)/(std::sqrt(41)/2);
        double Emean = 2*(E12 + E13 + E23)/3;
        double Efluct = std::pow(E12 + E13 - Emean, 2) + std::pow(E12 + E23 - Emean, 2) + std::pow(E13 + E23 - Emean, 2);
        Efluct = std::sqrt(Efluct/2);   // divided by (3-1) = 2 instead of 3 because of the adjusted variance estimator
        Efluct /= 2;    // double counting

        CHECK(packing.getParticleEnergyFluctuations(electrostaticInteraction) == Approx(Efluct));
    }

    // A method temporarily retired

    /*SECTION("particle energy") {
        CHECK(packing.calculateParticleEnergy(0, distanceInteraction) == Approx((2 + std::sqrt(41)) / 10 * 5));
        CHECK(packing.calculateParticleEnergy(1, distanceInteraction) == Approx((2 + std::sqrt(41)) / 10 * 5));
        CHECK(packing.calculateParticleEnergy(2, distanceInteraction) == Approx(2 * std::sqrt(41) / 10 * 5));
    }*/
}

TEST_CASE("Packing: multiple interaction center moves") {
    // Packing has linear scale of 5, so all coordinates in translate are multiplied by 3 (before scaling of course)
    double radius = 0.5;
    DimerHardCoreInteraction hardCore(radius);
    DimerDistanceInteraction distanceInteraction{};
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes;
    shapes.emplace_back(Vector<3>{0.5, 0.5, 0.5});
    shapes.emplace_back(Vector<3>{0.5, 3.5, 0.5});
    Packing packing({5, 5, 5}, std::move(shapes), std::move(pbc), hardCore);

    constexpr double inf = std::numeric_limits<double>::infinity();

    SECTION ("scaling") {
        SECTION("hard core without overlapping") {
            // For scale 0.5 dimers 0 and 1 are touching (through pbc). So a bit more should prevent any overlaps
            CHECK(packing.tryScaling(0.51, hardCore) == 0);
            CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.255, 0.255, 0.255}, {0.255, 1.785, 0.255}}, 1e-9));
        }

        SECTION("hard core downward with overlapping") {
            // Same as above, but a little bit more gives an overlap
            REQUIRE(packing.tryScaling(0.49, hardCore) == inf);

            SECTION("reverting the move") {
                packing.revertScaling();
                CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.5, 0.5, 0.5}, {0.5, 3.5, 0.5}}, 1e-9));
            }
        }

        SECTION("distance interaction") {
            packing.setupForInteraction(distanceInteraction);
            // interaction: particle1.center1 <-> particle2.center2
            // before scaling:
            // 0.0 <-> 1.0: d = 2
            // 0.0 <-> 1.1: d = sqrt(2^2 + 1^2) = sqrt(5)
            // 1.0 <-> 1.0: d = sqrt(2^2 + 1^2) = sqrt(5)
            // 1.0 <-> 1.1: d = 2
            // after scaling:
            // 0.0 <-> 1.0: d = 1
            // 0.0 <-> 1.1: d = sqrt(1^2 + 1^2) = sqrt(2)
            // 1.0 <-> 1.0: d = sqrt(1^2 + 1^2) = sqrt(2)
            // 1.0 <-> 1.1: d = 1
            // We scale downward from 5 to 2.5
            double scale1E = (4 + 2*std::sqrt(5));
            double scale05E = (2 + 2*std::sqrt(2));
            double dE = scale05E - scale1E;
            CHECK(packing.tryScaling(0.5, distanceInteraction) == Approx(dE));
        }
    }

    SECTION("translating") {
        SECTION("non-overlapping") {
            // Translation {0, 1, 0} places particle 1 at {0.5, 4.5, 0.5}, while particle 1 is at
            // {0.5, 0.5, 0.5} - they touch through pbc on y coordinate. Do a little bit less prevents overlap
            CHECK(packing.tryTranslation(1, {0, 0.9, 0}, hardCore) == 0);
            CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.5, 0.5, 0.5}, {0.5, 3.5, 0.5}}, 1e-9));
            SECTION("accepting the move") {
                packing.acceptTranslation();
                CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.5, 0.5, 0.5}, {0.5, 4.4, 0.5}}, 1e-9));
            }
        }

        SECTION("overlapping") {
            // Same as above, but we do more instead of less
            CHECK(packing.tryTranslation(1, {0, 1.1, 0}, hardCore) == inf);
            CHECK_THAT(packing, HasParticlesWithApproxPositions({{0.5, 0.5, 0.5}, {0.5, 3.5, 0.5}}, 1e-9));
        }

        SECTION("distance interaction") {
            packing.setupForInteraction(distanceInteraction);
            // interaction: particle1.center1 <-> particle2.center2
            // before translation:
            // 0.0 <-> 1.0: d = 2
            // 0.0 <-> 1.1: d = sqrt(2^2 + 1^2) = sqrt(5)
            // 1.0 <-> 1.0: d = sqrt(2^2 + 1^2) = sqrt(5)
            // 1.0 <-> 1.1: d = 2
            // after translation:
            // 0.0 <-> 1.0: d = 1
            // 0.0 <-> 1.1: d = sqrt(1^2 + 1^2) = sqrt(2)
            // 1.0 <-> 1.0: d = sqrt(1^2 + 1^2) = sqrt(2)
            // 1.0 <-> 1.1: d = 1
            double E0 = (4 + 2*std::sqrt(5));
            double E1 = (2 + 2*std::sqrt(2));
            double dE = E1 - E0;
            CHECK(packing.tryTranslation(1, {0, 1, 0}, distanceInteraction) == Approx(dE));

            SECTION("correct energy before and after accepted move") {
                CHECK(packing.getTotalEnergy(distanceInteraction) == Approx(E0));
                packing.acceptTranslation();
                CHECK(packing.getTotalEnergy(distanceInteraction) == Approx(E1));
            }
        }
    }
}

TEST_CASE("Packing: hard single interaction center overlap counting") {
    using Radius = PolydisperseSphereHardCoreInteraction::Radius;
    Radius radius = 0.5;
    PolydisperseSphereHardCoreInteraction hardCore(radius);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes;
    auto noRot = Matrix<3, 3>::identity();
    shapes.emplace_back(Vector<3>{0.5, 0.5, 0.5}, noRot, radius);
    shapes.emplace_back(Vector<3>{0.5, 0.5, 1.0}, noRot, radius);
    shapes.emplace_back(Vector<3>{0.5, 0.5, 2.25}, noRot, radius);
    Packing packing({5, 5, 5}, std::move(shapes), std::move(pbc), hardCore);
    packing.toggleOverlapCounting(true, hardCore);

    REQUIRE(packing.getCachedNumberOfOverlaps() == 1);

    constexpr double INF = std::numeric_limits<double>::infinity();

    SECTION("translation increasing overlaps") {
        CHECK(packing.tryTranslation(1, {0, 0, 0.3}, hardCore) == INF);
        CHECK(packing.getCachedNumberOfOverlaps() == 1);
        packing.acceptTranslation();
        CHECK(packing.getCachedNumberOfOverlaps() == 2);

        SECTION("opposite move") {
            CHECK(packing.tryTranslation(1, {0, 0, -0.3}, hardCore) == -INF);
            CHECK(packing.getCachedNumberOfOverlaps() == 2);
            packing.acceptTranslation();
            CHECK(packing.getCachedNumberOfOverlaps() == 1);
        }
    }

    SECTION("translation decreasing overlaps") {
        CHECK(packing.tryTranslation(1, {0, 3, 0}, hardCore) == -INF);
        CHECK(packing.getCachedNumberOfOverlaps() == 1);
        packing.acceptTranslation();
        CHECK(packing.getCachedNumberOfOverlaps() == 0);

        SECTION("opposite move") {
            CHECK(packing.tryTranslation(1, {0, -3, 0}, hardCore) == INF);
            CHECK(packing.getCachedNumberOfOverlaps() == 0);
            packing.acceptTranslation();
            CHECK(packing.getCachedNumberOfOverlaps() == 1);
        }
    }

    SECTION("translation preserving overlaps") {
        CHECK(packing.tryTranslation(1, {0, 0, 0.1}, hardCore) == 0);
        packing.acceptTranslation();
        CHECK(packing.getCachedNumberOfOverlaps() == 1);
    }

    SECTION("scaling increasing overlaps") {
        CHECK(packing.tryScaling(0.5, hardCore) == INF);
        CHECK(packing.getCachedNumberOfOverlaps() == 3);

        SECTION("reverting") {
            packing.revertScaling();
            CHECK(packing.getCachedNumberOfOverlaps() == 1);
        }

        SECTION("opposite move") {
            CHECK(packing.tryScaling(2, hardCore) == -INF);
            CHECK(packing.getCachedNumberOfOverlaps() == 1);
        }
    }

    SECTION("scaling decreasing overlaps") {
        CHECK(packing.tryScaling(3, hardCore) == -INF);
        CHECK(packing.getCachedNumberOfOverlaps() == 0);

        SECTION("reverting") {
            packing.revertScaling();
            CHECK(packing.getCachedNumberOfOverlaps() == 1);
        }

        SECTION("opposite move") {
            CHECK(packing.tryScaling(1./3, hardCore) == INF);
            CHECK(packing.getCachedNumberOfOverlaps() == 1);
        }
    }

    SECTION("scaling preserving overlaps") {
        CHECK(packing.tryScaling(1.05, hardCore) == 0);
        CHECK(packing.getCachedNumberOfOverlaps() == 1);
    }
}

TEST_CASE("Packing: multiple interaction center overlap counting") {
    double radius = 0.5;
    DimerHardCoreInteraction hardCore(radius);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes;
    shapes.emplace_back(Vector<3>{0.5, 0.5, 0.5});
    shapes.emplace_back(Vector<3>{1.5, 1, 0.5});
    Packing packing({5, 5, 5}, std::move(shapes), std::move(pbc), hardCore);

    packing.toggleOverlapCounting(true, hardCore);

    REQUIRE(packing.getCachedNumberOfOverlaps() == 1);

    constexpr double INF = std::numeric_limits<double>::infinity();

    SECTION("translation increasing overlaps") {
        CHECK(packing.tryTranslation(1, {-0.5, 0, 0}, hardCore) == INF);
        CHECK(packing.getCachedNumberOfOverlaps() == 1);
        packing.acceptTranslation();
        CHECK(packing.getCachedNumberOfOverlaps() == 3);

        SECTION("opposite move") {
            CHECK(packing.tryTranslation(1, {0.5, 0, 0}, hardCore) == -INF);
            CHECK(packing.getCachedNumberOfOverlaps() == 3);
            packing.acceptTranslation();
            CHECK(packing.getCachedNumberOfOverlaps() == 1);
        }
    }

    SECTION("translation decreasing overlaps") {
        CHECK(packing.tryTranslation(1, {0, 1, 0}, hardCore) == -INF);
        CHECK(packing.getCachedNumberOfOverlaps() == 1);
        packing.acceptTranslation();
        CHECK(packing.getCachedNumberOfOverlaps() == 0);

        SECTION("opposite move") {
            CHECK(packing.tryTranslation(1, {0, -1, 0}, hardCore) == INF);
            CHECK(packing.getCachedNumberOfOverlaps() == 0);
            packing.acceptTranslation();
            CHECK(packing.getCachedNumberOfOverlaps() == 1);
        }
    }

    SECTION("translation preserving overlaps") {
        CHECK(packing.tryTranslation(1, {0.1, 0, 0}, hardCore) == 0);
        packing.acceptTranslation();
        CHECK(packing.getCachedNumberOfOverlaps() == 1);
    }

    SECTION("scaling increasing overlaps") {
        CHECK(packing.tryScaling(0.5, hardCore) == INF);
        CHECK(packing.getCachedNumberOfOverlaps() == 3);

        SECTION("reverting") {
            packing.revertScaling();
            CHECK(packing.getCachedNumberOfOverlaps() == 1);
        }

        SECTION("opposite move") {
            CHECK(packing.tryScaling(2, hardCore) == -INF);
            CHECK(packing.getCachedNumberOfOverlaps() == 1);
        }
    }

    SECTION("scaling decreasing overlaps") {
        CHECK(packing.tryScaling(2, hardCore) == -INF);
        CHECK(packing.getCachedNumberOfOverlaps() == 0);

        SECTION("reverting") {
            packing.revertScaling();
            CHECK(packing.getCachedNumberOfOverlaps() == 1);
        }

        SECTION("opposite move") {
            CHECK(packing.tryScaling(0.5, hardCore) == INF);
            CHECK(packing.getCachedNumberOfOverlaps() == 1);
        }
    }

    SECTION("scaling preserving overlaps") {
        CHECK(packing.tryScaling(1.05, hardCore) == 0);
        CHECK(packing.getCachedNumberOfOverlaps() == 1);
    }
}

#include <iostream>

TEST_CASE("Packing: single interaction centre wall overlap") {
    auto scalingThreads = GENERATE(1, 2);

    DYNAMIC_SECTION("scaling threads: " << scalingThreads) {
        using Radius = PolydisperseSphereHardCoreInteraction::Radius;
        PolydisperseSphereHardCoreInteraction hardCore(Radius{0.6});
        auto pbc = std::make_unique<PeriodicBoundaryConditions>();
        std::vector<Shape> shapes;
        auto noRot = Matrix<3, 3>::identity();
        shapes.emplace_back(Vector<3>{1, 1, 1}, noRot, Radius{0.6});
        shapes.emplace_back(Vector<3>{2.5, 2.5, 2.5}, noRot, Radius{0.5});
        shapes.emplace_back(Vector<3>{4, 4, 4}, noRot, Radius{0.4});
        Packing packing({5, 5, 5}, std::move(shapes), std::move(pbc), hardCore, 1, scalingThreads);
        packing.toggleWall(0, true);
        packing.toggleWall(1, false);
        packing.toggleWall(2, true);

        constexpr double INF = std::numeric_limits<double>::infinity();

        SECTION("without overlaps counting") {
            // Wall at coord 0
            CHECK(packing.tryTranslation(0, {0, 0, -0.3}, hardCore) == 0);
            CHECK(packing.tryTranslation(0, {0, 0, -0.5}, hardCore) == INF);
            CHECK(packing.tryTranslation(2, {0, 0, 0.5}, hardCore) == 0);
            CHECK(packing.tryTranslation(2, {0, 0, 0.7}, hardCore) == INF);

            // No wall at coord 1
            CHECK(packing.tryTranslation(0, {0, -0.5, 0}, hardCore) == 0);
            CHECK(packing.tryTranslation(2, {0, -0.7, 0}, hardCore) == 0);

            // Wall at coord 2
            CHECK(packing.tryTranslation(0, {-0.3, 0, 0}, hardCore) == 0);
            CHECK(packing.tryTranslation(0, {-0.5, 0, 0}, hardCore) == INF);
            CHECK(packing.tryTranslation(2, {0.5, 0, 0}, hardCore) == 0);
            CHECK(packing.tryTranslation(2, {0.7, 0, 0}, hardCore) == INF);
        }

        SECTION("with overlaps counting") {
            packing.toggleOverlapCounting(true, hardCore);

            // Move molecule 0 into a corner with 2 walls (new pos: {0.5, 1, 0.5}, overlaps: 2)
            CHECK(packing.tryTranslation(0, {-0.5, 0, -0.5}, hardCore) == INF);
            packing.acceptTranslation();
            CHECK(packing.getCachedNumberOfOverlaps() == 2);
            CHECK(packing.countTotalOverlaps(hardCore, false) == 2);
            std::cout << packing << std::endl;

            // Move molecule 0 into a corner to collide with only 1 wall (new pos: {0.7, 1, 0.5}, overlaps: 1)
            CHECK(packing.tryTranslation(0, {0.2, 0, 0}, hardCore) == -INF);
            packing.acceptTranslation();
            CHECK(packing.getCachedNumberOfOverlaps() == 1);
            CHECK(packing.countTotalOverlaps(hardCore, false) == 1);
            std::cout << packing << std::endl;

            // Move molecule 0 out of walls into molecule 1 (new pos: {2.5, 2.5, 1.5}, overlaps: 1)
            CHECK(packing.tryTranslation(0, {1.8, 1.5, 1.0}, hardCore) == 0);
            packing.acceptTranslation();
            CHECK(packing.getCachedNumberOfOverlaps() == 1);
            CHECK(packing.countTotalOverlaps(hardCore, false) == 1);
            std::cout << packing << std::endl;
        }
    }
}

TEST_CASE("Packing: multiple interaction centres wall overlap") {
    double radius = 0.5;
    DimerHardCoreInteraction hardCore(radius);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes;
    shapes.emplace_back(Vector<3>{1, 1, 1});
    shapes.emplace_back(Vector<3>{3, 3, 3});
    Packing packing({5, 5, 5}, std::move(shapes), std::move(pbc), hardCore);
    packing.toggleWall(0, true);
    packing.toggleWall(2, true);

    constexpr double INF = std::numeric_limits<double>::infinity();

    SECTION("without overlaps counting") {
        CHECK(packing.tryTranslation(0, {-0.4, 0, 0}, hardCore) == 0);
        CHECK(packing.tryTranslation(0, {-0.6, 0, 0}, hardCore) == INF);
        CHECK(packing.tryTranslation(1, {0.4, 0, 0}, hardCore) == 0);
        CHECK(packing.tryTranslation(1, {0.6, 0, 0}, hardCore) == INF);
    }

    SECTION("with overlaps counting") {
        packing.toggleOverlapCounting(true, hardCore);

        // Move molecule into a corner with 2 walls (new pos: {3.6, 1, 4.6}, overlaps: 3)
        CHECK(packing.tryTranslation(0, {2.6, 0, 3.6}, hardCore) == INF);
        packing.acceptTranslation();
        CHECK(packing.getCachedNumberOfOverlaps() == 3);
        CHECK(packing.countTotalOverlaps(hardCore, false) == 3);
        // Move molecule into a corner to collide with only 1 wall (new pos: {3.6, 1, 1}, overlaps: 1)
        CHECK(packing.tryTranslation(0, {0, 0, -3.6}, hardCore) == -INF);
        packing.acceptTranslation();
        CHECK(packing.getCachedNumberOfOverlaps() == 1);
        CHECK(packing.countTotalOverlaps(hardCore, false) == 1);
        // Move molecule out of walls into another molecule (new pos: {1.5, 3, 3}, overlaps: 2)
        CHECK(packing.tryTranslation(0, {3.1, 2, 2}, hardCore) == 0);
        packing.acceptTranslation();
        CHECK(packing.getCachedNumberOfOverlaps() == 1);
        CHECK(packing.countTotalOverlaps(hardCore, false) == 1);
    }
}

TEST_CASE("Packing: too big NG cell bug") {
    // Previous behaviour:
    // 100 x 100 x 1.1 packing forced too big NG cell - volume=11000, so the cell size set to give "at most 5^3 cells
    // per molecule" was more than the box height resulting in NG throwing exception that the cell is too big.
    //
    // New, correct behaviour:
    // If that happens, neighbour grid is prevented from being created.

    double radius = 0.5;
    SphereHardCoreInteraction hardCore(radius);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes;
    shapes.emplace_back(Vector<3>{0.5, 25, 25});
    shapes.emplace_back(Vector<3>{0.5, 75, 75});

    REQUIRE_NOTHROW(Packing({1.1, 100, 100}, std::move(shapes), std::move(pbc), hardCore));
}

TEST_CASE("Packing: named points dumping") {
    double radius = 0.5;
    SphereHardCoreInteraction hardCore(radius);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes{Shape{{0.5, 0.5, 0.5}}, Shape{{0.5, 3.5, 0.5}, Matrix<3, 3>::rotation(0, 0, M_PI/2)}};
    Packing packing({5, 5, 5}, std::move(shapes), std::move(pbc), hardCore);
    using trompeloeil::_;
    MockShapeGeometry geometry;
    geometry.addNamedPoints({{"point", {1, 0, 0}}});

    auto points = packing.dumpNamedPoints(geometry, "point");

    REQUIRE(points.size() == 2);
    CHECK_THAT(points[0], IsApproxEqual({1.5, 0.5, 0.5}, 1e-12));
    CHECK_THAT(points[1], IsApproxEqual({0.5, 4.5, 0.5}, 1e-12));
}
