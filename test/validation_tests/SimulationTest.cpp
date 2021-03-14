//
// Created by Piotr Kubala on 14/12/2020.
//

#include <catch2/catch.hpp>
#include <sstream>

#include "core/Simulation.h"
#include "core/LatticeArrangingModel.h"
#include "core/shapes/SphereTraits.h"
#include "core/shapes/SpherocylinderTraits.h"
#include "core/shapes/KMerTraits.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/interactions/LennardJonesInteraction.h"
#include "core/interactions/RepulsiveLennardJonesInteraction.h"
#include "utils/OMPMacros.h"

TEST_CASE("Simulation: equilibration for dilute hard sphere gas", "[short]") {
    // We choose temperature 10 and pressure 1. For particles of radius 0.05 we should obtain number density 0.0999791
    // We start with density 0.01 and too small step ranges. The program should adjust and equilibrate correctly
    omp_set_num_threads(1);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 5000;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = LatticeArrangingModel{}.arrange(50, dimensions);
    SphereTraits sphereTraits(0.05);
    auto packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(pbc), sphereTraits.getInteraction());
    Simulation simulation(std::move(packing), 1, 0.1, 1, 1234);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.perform(10, 1, 5000, 10000, 100, sphereTraits.getInteraction(), logger);

    Quantity density = simulation.getAverageDensity();
    double expected = 0.0999791;
    INFO("Carnahan-Starling density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.03); // up to 3%
}

TEST_CASE("Simulation: degenerate hard sphere gas", "[short]") {
    omp_set_num_threads(1);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 200;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = LatticeArrangingModel{}.arrange(50, dimensions);
    SphereTraits sphereTraits(0.5);
    auto packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(pbc), sphereTraits.getInteraction());
    Simulation simulation(std::move(packing), 1, 0.1, 1, 1234);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.perform(1, 1, 5000, 10000, 100, sphereTraits.getInteraction(), logger);

    Quantity density = simulation.getAverageDensity();
    double expected = 0.398574;
    INFO("Carnahan-Starling density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.03); // up to 3%
}

TEST_CASE("Simulation: slightly degenerate hard spherocylinder gas", "[short]") {
    omp_set_num_threads(1);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 200;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = LatticeArrangingModel{}.arrange(50, dimensions);
    SpherocylinderTraits spherocylinderTraits(0.5, 0.2);
    auto packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(pbc), spherocylinderTraits.getInteraction());
    Simulation simulation(std::move(packing), 1, 0.1, 1, 1234);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.perform(10, 1, 5000, 10000, 100, spherocylinderTraits.getInteraction(), logger);

    Quantity density = simulation.getAverageDensity();
    double expected = 0.0956448;
    INFO("Boublik density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.03); // up to 3%
}

TEST_CASE("Simulation: slightly degenerate Lennard-Jones gas", "[short]") {
    // For parameters chosen compressibility factor should be around 1.2 and equation of state seem to be well
    // approximated by the second virial coefficient known analytically
    omp_set_num_threads(1);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 200;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = LatticeArrangingModel{}.arrange(50, dimensions);
    SphereTraits sphereTraits(0.5, std::make_unique<LennardJonesInteraction>(1, 0.5));
    auto packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(pbc), sphereTraits.getInteraction());
    // More frequent averaging here to preserve short simulation times (particle displacement are large anyway)
    Simulation simulation(std::move(packing), 1, 0.1, 10, 1234);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.perform(100, 200, 2000, 2000, 20, sphereTraits.getInteraction(), logger);

    Quantity density = simulation.getAverageDensity();
    double expected = 1.6637139014398628;
    INFO("1-st order virial density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.03); // up to 3%
}

TEST_CASE("Simulation: hard dumbbell fluid", "[short]") {
    // Semi-Theoretical values from "An equation of state for hard dumbell fluids"
    // D.J. Tildesley a & W.B. Streett (1980)
    omp_set_num_threads(1);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 500;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = LatticeArrangingModel{}.arrange(50, dimensions);
    KMerTraits kmerTraits(2, 0.5, 1);
    auto packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(pbc), kmerTraits.getInteraction());
    // More frequent averaging here to preserve short simulation times (particle displacement are large anyway)
    Simulation simulation(std::move(packing), 10, 1, 10, 1234);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.perform(1, 2, 10000, 5000, 100, kmerTraits.getInteraction(), logger);

    Quantity density = simulation.getAverageDensity();
    double expected = 0.3043317608769238;
    INFO("Tildesley-Streett density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.01); // up to 1%
}

TEST_CASE("Simulation: wca dumbbell fluid", "[medium]") {
    // Value for density remorselessly stolen from
    // https://github.com/glotzerlab/hoomd-blue/blob/master/hoomd/hpmc/validation/wca_dumbbell.py
    omp_set_num_threads(4);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 500;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = LatticeArrangingModel{}.arrange(50, dimensions);
    auto interaction = std::make_unique<RepulsiveLennardJonesInteraction>(1, 1);
    KMerTraits kmerTraits(2, 0.5, 1, std::move(interaction));
    auto packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(pbc), kmerTraits.getInteraction());
    // More frequent averaging here to preserve short simulation times (particle displacement are large anyway)
    Simulation simulation(std::move(packing), 10, 1, 10, 1234);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.perform(1, 7.5, 5000, 5000, 100, kmerTraits.getInteraction(), logger);

    Quantity density = simulation.getAverageDensity();
    double expected = 0.43451;
    INFO("hoomd-blue density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.01); // up to 1%
}