//
// Created by pkup on 18.06.2021.
//

#include <catch2/catch.hpp>

#include "matchers/MatrixApproxMatcher.h"

#include "core/lattice/DistanceOptimizer.h"

#include "core/shapes/SpherocylinderTraits.h"
#include "core/shapes/SphereTraits.h"
#include "core/lattice/Lattice.h"
#include "core/PeriodicBoundaryConditions.h"


TEST_CASE("DiscanceOptimizer: axis optimization") {
    SpherocylinderTraits scTraits(2, 1);
    Shape sc1, sc2;
    sc2.setOrientation(Matrix<3, 3>::rotation(0, M_PI/2, 0));

    SECTION("minimizeForDirection: x") {
        double distance = DistanceOptimizer::minimizeForDirection(sc1, sc2, {1, 0, 0}, scTraits.getInteraction());

        CHECK(distance == Approx(3).margin(DistanceOptimizer::EPSILON));
    }

    SECTION("minimizeForAxes") {
        auto distances = DistanceOptimizer::minimizeForAxes(sc1, sc2, scTraits.getInteraction());

        CHECK(distances[0] == Approx(3).margin(DistanceOptimizer::EPSILON));
        CHECK(distances[1] == Approx(2).margin(DistanceOptimizer::EPSILON));
        CHECK(distances[2] == Approx(3).margin(DistanceOptimizer::EPSILON));
    }
}

TEST_CASE("DistanceOptimizer: shrink packing - layer orthorhombic") {
    // BCC spherocylinders
    UnitCell unitCell(TriclinicBox(5), {Shape({0.25, 0.25, 0.25}), Shape({0.75, 0.75, 0.75})});
    Lattice lattice(unitCell, {4, 4, 2});
    auto shapes = lattice.generateMolecules();
    SpherocylinderTraits scTraits(1, 0.5);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing(lattice.getLatticeBox(), shapes, std::move(pbc), scTraits.getInteraction());

    DistanceOptimizer::shrinkPacking(packing, scTraits, "xyz");

    double Lx = 4;
    double Ly = 4;
    double Lz = 2*(2 + M_SQRT2);
    CHECK_THAT(packing.getBox().getDimensions(), IsApproxEqual(Matrix<3, 3>{Lx, 0, 0, 0, Ly, 0, 0, 0, Lz}, 1e-12));
}

TEST_CASE("DistanceOptimizer: shrink packing - hexagonal") {
    Matrix<3, 3> unitCellBox{1,              0.5, 0,
                             0, 0.5*std::sqrt(3), 0,
                             0,                0, 1};
    UnitCell unitCell(TriclinicBox(5. * unitCellBox), {Shape({0.5, 0.5, 0.5})});
    Lattice lattice(unitCell, {4, 4, 4});
    auto shapes = lattice.generateMolecules();
    SphereTraits sphereTraits(0.5);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing(lattice.getLatticeBox(), shapes, std::move(pbc), sphereTraits.getInteraction());

    DistanceOptimizer::shrinkPacking(packing, sphereTraits.getInteraction(), "xyz");

    CHECK_THAT(packing.getBox().getDimensions(), IsApproxEqual(4. * unitCellBox, 1e-12));
}