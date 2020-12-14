//
// Created by Piotr Kubala on 14/12/2020.
//

#include <catch2/catch.hpp>
#include <sstream>

#include "core/Simulation.h"
#include "core/LatticeArrangingModel.h"
#include "core/Sphere.h"
#include "core/PeriodicBoundaryConditions.h"

TEST_CASE("Simulation: equilibration from 0.1 density") {
    Sphere sphere(0.5);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 5000;
    double linearSize = std::cbrt(V);
    auto shapes = LatticeArrangingModel{}.arrange(sphere, 50, linearSize, *pbc);
    auto packing = std::make_unique<Packing>(linearSize, std::move(shapes), std::move(pbc));
    Simulation simulation(10, 1, 1, 1, 1000000, 10000, 1234);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.perform(std::move(packing), logger);

    REQUIRE(simulation.getAverageDensity() == Approx(0.1).margin(0.001));
}