//
// Created by Piotr Kubala on 14/12/2020.
//

#include <catch2/catch.hpp>
#include <sstream>

#include "core/Simulation.h"
#include "core/LatticeArrangingModel.h"
#include "core/shapes/SphereTraits.h"
#include "core/shapes/SpherocylinderTraits.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/LennardJonesInteraction.h"

TEST_CASE("Simulation: equilibration for dilute hard sphere gas") {
    // We choose temperature 10 and pressure 1. For particles of radius 0.05 we should obtain number density 0.0999791
    // We start with density 0.01 and too small step ranges. The program should adjust and equilibrate correctly
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 5000;
    double linearSize = std::cbrt(V);
    auto shapes = LatticeArrangingModel{}.arrange(50, 1);
    auto packing = std::make_unique<Packing>(linearSize, std::move(shapes), std::move(pbc));
    SphereTraits sphereTraits(0.05);
    Simulation simulation(10, 1, 1, 0.1, 1, 5000, 10000, 100, 1234);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.perform(std::move(packing), sphereTraits, logger);

    Quantity density = simulation.getAverageDensity();
    double expected = 0.0999791;
    INFO("Carnahan-Starling density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.03); // up to 3%
}

TEST_CASE("Simulation: degenerate hard sphere gas") {
    // We choose temperature 10 and pressure 1. For particles of radius 0.05 we should obtain number density 0.0999791
    // We start with density 0.01 and too small step ranges. The program should adjust and equilibrate correctly
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 200;
    double linearSize = std::cbrt(V);
    auto shapes = LatticeArrangingModel{}.arrange(50, 1);
    auto packing = std::make_unique<Packing>(linearSize, std::move(shapes), std::move(pbc));
    SphereTraits sphereTraits(0.5);
    Simulation simulation(1, 1, 1, 0.1, 1, 5000, 10000, 100, 1234);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.perform(std::move(packing), sphereTraits.getInteraction(), logger);

    Quantity density = simulation.getAverageDensity();
    double expected = 0.398574;
    INFO("Carnahan-Starling density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.03); // up to 3%
}

TEST_CASE("Simulation: slightly degenerate hard spherocylinder gas") {
    // We choose temperature 10 and pressure 1. For particles of radius 0.05 we should obtain number density 0.0999791
    // We start with density 0.01 and too small step ranges. The program should adjust and equilibrate correctly

    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 200;
    double linearSize = std::cbrt(V);
    auto shapes = LatticeArrangingModel{}.arrange(50, 1);
    auto packing = std::make_unique<Packing>(linearSize, std::move(shapes), std::move(pbc));
    SpherocylinderTraits spherocylinderTraits(0.5, 0.2);
    Simulation simulation(10, 1, 1, 0.1, 1, 5000, 10000, 100, 1234);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.perform(std::move(packing), spherocylinderTraits.getInteraction(), logger);

    Quantity density = simulation.getAverageDensity();
    double expected = 0.0956448;
    INFO("Boublik density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.03); // up to 3%
}

TEST_CASE("Simulation: slightly degenerate Lennard-Jones gas") {
    // For parameters chosen compressibility factor should be around 1.2 and equation of state seem to be well
    // approximated by the second virial coefficient known analytically
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 200;
    double linearSize = std::cbrt(V);
    auto shapes = LatticeArrangingModel{}.arrange(50, 1);
    auto packing = std::make_unique<Packing>(linearSize, std::move(shapes), std::move(pbc));
    SphereTraits sphereTraits(0.5, std::make_unique<LennardJonesInteraction>(1, 0.5));
    // More frequent averaging here to preserve short simulation times (particle displacement are large anyway)
    Simulation simulation(100, 200, 1, 0.1, 10, 1000, 1000, 10, 1234);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.perform(std::move(packing), sphereTraits.getInteraction(), logger);

    Quantity density = simulation.getAverageDensity();
    double expected = 1.6637139014398628;
    INFO("1-st order virial density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.03); // up to 3%
}