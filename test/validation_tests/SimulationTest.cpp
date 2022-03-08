//
// Created by Piotr Kubala on 14/12/2020.
//

#include <catch2/catch.hpp>
#include <sstream>

#include "core/Simulation.h"
#include "core/arranging_models/OrthorhombicArrangingModel.h"
#include "core/shapes/SphereTraits.h"
#include "core/shapes/SpherocylinderTraits.h"
#include "core/shapes/KMerTraits.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/interactions/LennardJonesInteraction.h"
#include "core/interactions/RepulsiveLennardJonesInteraction.h"
#include "core/volume_scalers/DeltaVolumeScaler.h"
#include "core/volume_scalers/TriclinicAdapter.h"
#include "core/ObservablesCollector.h"
#include "core/observables/NumberDensity.h"
#include "core/shapes/CompoundShapeTraits.h"
#include "core/interactions/SquareInverseCoreInteraction.h"
#include "utils/OMPMacros.h"


TEST_CASE("Simulation: equilibration for dilute hard sphere gas", "[short]") {
    // We choose temperature 10 and pressure 1. For particles of radius 0.05 we should obtain number density 0.0999791
    // We start with density 0.01 and too small step ranges. The program should adjust and equilibrate correctly
    omp_set_num_threads(1);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 5000;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = OrthorhombicArrangingModel{}.arrange(50, dimensions);
    SphereTraits sphereTraits(0.05);
    auto packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(pbc), sphereTraits.getInteraction());
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>());
    Simulation simulation(std::move(packing), 1, 0.1, 1, 1234, std::move(volumeScaler));
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(10, 1, 5000, 10000, 100, 100, sphereTraits, std::move(collector), logger);

    Quantity density = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;
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
    auto shapes = OrthorhombicArrangingModel{}.arrange(50, dimensions);
    SphereTraits sphereTraits(0.5);
    auto packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(pbc), sphereTraits.getInteraction());
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>());;
    Simulation simulation(std::move(packing), 1, 0.1, 1, 1234, std::move(volumeScaler));
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(1, 1, 5000, 10000, 100, 100, sphereTraits, std::move(collector), logger);

    Quantity density = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;
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
    auto shapes = OrthorhombicArrangingModel{}.arrange(50, dimensions);
    SpherocylinderTraits spherocylinderTraits(0.5, 0.2);
    auto packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(pbc), spherocylinderTraits.getInteraction());
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>());
    Simulation simulation(std::move(packing), 1, 0.1, 1, 1234, std::move(volumeScaler));
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(10, 1, 5000, 10000, 100, 100, spherocylinderTraits, std::move(collector), logger);

    Quantity density = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;
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
    auto shapes = OrthorhombicArrangingModel{}.arrange(50, dimensions);
    SphereTraits sphereTraits(0.5, std::make_unique<LennardJonesInteraction>(1, 0.5));
    auto packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(pbc), sphereTraits.getInteraction());
    // More frequent averaging here to preserve short simulation times (particle displacement are large anyway)
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>());
    Simulation simulation(std::move(packing), 1, 0.1, 1, 1234, std::move(volumeScaler));
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(100, 200, 2000, 2000, 20, 20, sphereTraits, std::move(collector), logger);

    Quantity density = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;
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
    auto shapes = OrthorhombicArrangingModel{}.arrange(50, dimensions);
    KMerTraits kmerTraits(2, 0.5, 1);
    auto packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(pbc), kmerTraits.getInteraction());
    // More frequent averaging here to preserve short simulation times (particle displacement are large anyway)
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>());
    Simulation simulation(std::move(packing), 10, 1, 10, 1234, std::move(volumeScaler));
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(1, 2, 10000, 5000, 100, 100, kmerTraits, std::move(collector), logger);

    Quantity density = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;
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
    auto shapes = OrthorhombicArrangingModel{}.arrange(50, dimensions);
    auto interaction = std::make_unique<RepulsiveLennardJonesInteraction>(1, 1);
    KMerTraits kmerTraits(2, 0.5, 1, std::move(interaction));
    auto packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(pbc), kmerTraits.getInteraction());
    // More frequent averaging here to preserve short simulation times (particle displacement are large anyway)
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>());
    Simulation simulation(std::move(packing), 10, 1, 10, 1234, std::move(volumeScaler));
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(1, 7.5, 5000, 5000, 100, 100, kmerTraits, std::move(collector), logger);

    Quantity density = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;
    double expected = 0.43451;
    INFO("hoomd-blue density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.01); // up to 1%
}

TEST_CASE("Simulation: hard sphere domain decomposition", "[medium]") {
    omp_set_num_threads(4);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 1000;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = OrthorhombicArrangingModel{}.arrange(200, dimensions);
    SphereTraits sphereTraits(0.5);
    auto packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(pbc),
                                             sphereTraits.getInteraction(), 4, 4);
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>());
    Simulation simulation(std::move(packing), 1, 0.1, 1, 1234, std::move(volumeScaler), {2, 2, 1});
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(1, 1, 10000, 15000, 1000, 1000, sphereTraits, std::move(collector), logger);

    Quantity density = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;
    double expected = 0.398574;
    INFO("Carnahan-Starling density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.03); // up to 3%
}

TEST_CASE("Simulation: overlap reduction for hard sphere liquid", "[medium]") {
    auto domainDivisions = GENERATE(std::array<std::size_t, 3>{1, 1, 1}, std::array<std::size_t, 3>{2, 2, 1});
    std::size_t numDomains = std::accumulate(domainDivisions.begin(), domainDivisions.end(), 1, std::multiplies<>{});

    DYNAMIC_SECTION("domains: " << numDomains) {
        omp_set_num_threads(1);
        auto pbc = std::make_unique<PeriodicBoundaryConditions>();
        double linearSize = 5;
        std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
        auto shapes = OrthorhombicArrangingModel{}.arrange(216, dimensions);    // 6 x 6 x 6

        auto sphereTraits = std::make_unique<SphereTraits>(0.5);
        auto squareInverseCore = std::make_unique<SquareInverseCoreInteraction>(1, 1);
        auto helperSphereTraits = std::make_unique<SphereTraits>(0.5, std::move(squareInverseCore));
        CompoundShapeTraits compoundSphere(std::move(sphereTraits), std::move(helperSphereTraits));

        auto packing = std::make_unique<Packing>(dimensions, std::move(shapes), std::move(pbc),
                                                 compoundSphere.getInteraction(), numDomains, numDomains);
        auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>());
        Simulation simulation(std::move(packing), 0.1, 0.1, 1, 1234, std::move(volumeScaler), domainDivisions);
        auto collector = std::make_unique<ObservablesCollector>();
        std::ostringstream loggerStream;
        Logger logger(loggerStream);

        simulation.relaxOverlaps(1, 5, 1000, compoundSphere, std::move(collector), logger);

        double finalDensity = simulation.getPacking().getNumberDensity();
        std::size_t finalOverlaps = simulation.getPacking().getCachedNumberOfOverlaps();
        // 0.754 is equilibrium density - overlap reduction shouldn't overexpand the packing
        double minimalDensity = 0.75;
        INFO("Minimal density: " << minimalDensity);
        INFO("Density after overlap reduction: " << finalDensity);
        INFO("Overlaps afterwards: " << finalOverlaps);
        CHECK(finalOverlaps == 0);
        CHECK(finalDensity > minimalDensity);
    }
}