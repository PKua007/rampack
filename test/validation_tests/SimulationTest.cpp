//
// Created by Piotr Kubala on 14/12/2020.
//

#include <catch2/catch.hpp>
#include <sstream>

#include "core/Simulation.h"
#include "core/LatticeArrangingModel.h"
#include "core/shapes/Sphere.h"
#include "core/PeriodicBoundaryConditions.h"

TEST_CASE("Simulation: equilibration for dilute hard sphere gas") {
    // We choose temperature 10 and pressure 1. For particles of radius 0.05 we should obtain number density 0.0999791
    // We start with density 0.01 and too small step ranges. The program should adjust and equilibrate correctly
    Sphere sphere(0.05);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 5000;
    double linearSize = std::cbrt(V);
    auto shapes = LatticeArrangingModel{}.arrange(sphere, 50, linearSize, *pbc);
    auto packing = std::make_unique<Packing>(linearSize, std::move(shapes), std::move(pbc));
    Simulation simulation(10, 1, 1, 0.1, 1, 10000, 10000, 100, 1234);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.perform(std::move(packing), logger);

    Quantity density = simulation.getAverageDensity();
    double expected = 0.0999791;
    INFO("Carnahan-Starling density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.03); // up to 3%
}