//
// Created by Piotr Kubala on 14/12/2020.
//

#include <catch2/catch.hpp>
#include <sstream>

#include "core/Simulation.h"
#include "core/lattice/OrthorhombicArrangingModel.h"
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
#include "core/observables/NematicOrder.h"
#include "core/observables/PackingFraction.h"
#include "core/shapes/CompoundShapeTraits.h"
#include "core/interactions/SquareInverseCoreInteraction.h"
#include "core/move_samplers/RototranslationSampler.h"
#include "core/move_samplers/TranslationSampler.h"
#include "core/move_samplers/RotationSampler.h"
#include "utils/OMPMacros.h"
#include "core/lattice/UnitCellFactory.h"
#include "core/lattice/Lattice.h"
#include "core/volume_scalers/TriclinicDeltaScaler.h"


namespace {
    class OverlapGuard : public Observable {
    public:
        void calculate(const Packing &packing, [[maybe_unused]] double temperature, [[maybe_unused]] double pressure,
                       const ShapeTraits &shapeTraits) override
        {
            Assert(packing.countTotalOverlaps(shapeTraits.getInteraction()) == 0);
        }

        [[nodiscard]] std::vector<std::string> getIntervalHeader() const override { return {}; }
        [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
        [[nodiscard]] std::vector<double> getIntervalValues() const override { return {}; }
        [[nodiscard]] std::vector<std::string> getNominalValues() const override { return {}; }
        [[nodiscard]] std::string getName() const override { return "overlap guard"; }
    };
}

TEST_CASE("Simulation: equilibration for dilute hard sphere gas", "[short]") {
    // We choose temperature 10 and pressure 1. For particles of radius 0.05 we should obtain number density 0.0999791
    // We start with density 0.01 and too small step ranges. The program should adjust and equilibrate correctly
    OMP_SET_NUM_THREADS(1);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 5000;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = OrthorhombicArrangingModel{}.arrange(50, dimensions);
    SphereTraits sphereTraits(0.05);
    auto packing = std::make_unique<Packing>(
        dimensions, std::move(shapes), std::move(pbc), sphereTraits.getInteraction(), sphereTraits.getDataManager()
    );
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>(), 1);
    Simulation simulation(std::move(packing), 1, 0.1, 1234, std::move(volumeScaler));
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(10, 1, 5000, 10000, 100, 100, sphereTraits, std::move(collector), {}, logger);

    Quantity density = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;
    double expected = 0.0999791;
    INFO("Carnahan-Starling density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.03); // up to 3%
}

TEST_CASE("Simulation: degenerate hard sphere gas", "[short]") {
    OMP_SET_NUM_THREADS(1);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 200;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = OrthorhombicArrangingModel{}.arrange(50, dimensions);
    SphereTraits sphereTraits(0.5);
    auto packing = std::make_unique<Packing>(
        dimensions, std::move(shapes), std::move(pbc), sphereTraits.getInteraction(), sphereTraits.getDataManager()
    );
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>(), 1);
    Simulation simulation(std::move(packing), 1, 0.1, 1234, std::move(volumeScaler));
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(1, 1, 5000, 10000, 100, 100, sphereTraits, std::move(collector), {}, logger);

    Quantity density = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;
    double expected = 0.398574;
    INFO("Carnahan-Starling density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.03); // up to 3%
}

TEST_CASE("Simulation: slightly degenerate hard spherocylinder gas", "[short]") {
    OMP_SET_NUM_THREADS(1);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::array<double, 3> dimensions = {10, 10, 10};
    auto shapes = OrthorhombicArrangingModel{}.arrange(50, {5, 5, 2}, {2, 2, 5}, dimensions);
    SpherocylinderTraits spherocylinderTraits;
    for (auto &shape : shapes)
        shape.setData(SpherocylinderTraits::Data{3, 0.5});
    auto packing = std::make_unique<Packing>(
        dimensions, std::move(shapes), std::move(pbc), spherocylinderTraits.getInteraction(),
        spherocylinderTraits.getDataManager()
    );
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>(), 10);
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    // Value from previous simulations, but consulted with 10.1063/1.471343. The values from the paper could not be used
    // because of finite size effects for such a small system size as used here (to keep test runtime short enough).
    double expected = 0.09454087;    // mean standard error respecting correlation time: 0.00005054

    SECTION("rototranslation moves") {
        std::vector<std::unique_ptr<MoveSampler>> moveSamplers;
        moveSamplers.push_back(std::make_unique<RototranslationSampler>(0.5, 1));
        Simulation simulation(std::move(packing), std::move(moveSamplers), 1234, std::move(volumeScaler));

        simulation.integrate(1, 0.5, 5000, 10000, 1000, 100, spherocylinderTraits, std::move(collector), {}, logger);

        Quantity density = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;
        INFO("Boublik density: " << expected);
        INFO("Monte Carlo density: " << density);
        CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
        CHECK(density.error / density.value < 0.03); // up to 3%
    }

    SECTION("translation + rotation moves") {
        std::vector<std::unique_ptr<MoveSampler>> moveSamplers;
        moveSamplers.push_back(std::make_unique<TranslationSampler>(0.5));
        moveSamplers.push_back(std::make_unique<RotationSampler>(1));
        Simulation simulation(std::move(packing), std::move(moveSamplers), 1234, std::move(volumeScaler));

        simulation.integrate(1, 0.5, 5000, 10000, 1000, 100, spherocylinderTraits, std::move(collector), {}, logger);

        Quantity density = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;
        INFO("Boublik density: " << expected);
        INFO("Monte Carlo density: " << density);
        CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
        CHECK(density.error / density.value < 0.03); // up to 3%
    }
}

TEST_CASE("Simulation: slightly degenerate Lennard-Jones gas", "[short]") {
    // For parameters chosen compressibility factor should be around 1.2 and equation of state seem to be well
    // approximated by the second virial coefficient known analytically
    OMP_SET_NUM_THREADS(1);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 200;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = OrthorhombicArrangingModel{}.arrange(50, dimensions);
    SphereTraits sphereTraits(0.5, std::make_unique<LennardJonesInteraction>(1, 0.5));
    auto packing = std::make_unique<Packing>(
        dimensions, std::move(shapes), std::move(pbc), sphereTraits.getInteraction(), sphereTraits.getDataManager()
    );
    // More frequent averaging here to preserve short simulation times (particle displacement are large anyway)
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>(), 1);
    Simulation simulation(std::move(packing), 1, 0.1, 1234, std::move(volumeScaler));
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(100, 200, 2000, 2000, 20, 20, sphereTraits, std::move(collector), {}, logger);

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
    OMP_SET_NUM_THREADS(1);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 600;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = OrthorhombicArrangingModel{}.arrange(50, dimensions);
    KMerTraits kmerTraits(2, 0.5, 1);
    auto packing = std::make_unique<Packing>(
        dimensions, std::move(shapes), std::move(pbc), kmerTraits.getInteraction(), kmerTraits.getDataManager()
    );
    // More frequent averaging here to preserve short simulation times (particle displacement are large anyway)
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>(), 10);
    Simulation simulation(std::move(packing), 10, 1, 1234, std::move(volumeScaler));
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(1, 2, 20000, 20000, 1000, 100, kmerTraits, std::move(collector), {}, logger);

    Quantity density = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;
    double expected = 0.3043317608769238;
    INFO("Tildesley-Streett density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.01); // up to 1%
}

TEST_CASE("Simulation: wca dumbbell fluid", "[medium]") {
    OMP_SET_NUM_THREADS(4);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 500;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = OrthorhombicArrangingModel{}.arrange(50, dimensions);
    // TODO: fix is properly
    for (auto &shape : shapes)
        shape.rotate(Matrix<3, 3>::rotation(0, M_PI/2, 0));
    auto interaction = std::make_unique<RepulsiveLennardJonesInteraction>(1, 1);
    KMerTraits kmerTraits(2, 0.5, 1, std::move(interaction));
    auto packing = std::make_unique<Packing>(
        dimensions, std::move(shapes), std::move(pbc), kmerTraits.getInteraction(), kmerTraits.getDataManager()
    );
    // More frequent averaging here to preserve short simulation times (particle displacement are large anyway)
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>(), 15);
    Simulation simulation(std::move(packing), 0.5, 0.15, 1234, std::move(volumeScaler));
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(1, 7.5, 15000, 15000, 100, 100, kmerTraits, std::move(collector), {}, logger);

    Quantity density = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;

    // Old value for density remorselessly stolen from
    // https://github.com/glotzerlab/hoomd-blue/blob/master/hoomd/hpmc/validation/wca_dumbbell.py
    // The value is correct for larger systems - here finite size effects change it slightly
    //double expected = 0.43451;

    // Value calculated in previous simulations - acts as a regression test
    double expected = 0.43503541;
    INFO("hoomd-blue density: " << expected);
    INFO("Monte Carlo density: " << density);
    CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
    CHECK(density.error / density.value < 0.01); // up to 1%
}

TEST_CASE("Simulation: hard sphere domain decomposition", "[medium]") {
    OMP_SET_NUM_THREADS(4);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 1000;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = OrthorhombicArrangingModel{}.arrange(200, dimensions);
    SphereTraits sphereTraits(0.5);
    auto packing = std::make_unique<Packing>(
        dimensions, std::move(shapes), std::move(pbc), sphereTraits.getInteraction(), sphereTraits.getDataManager(), 4, 4
    );
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>(), 1);
    Simulation simulation(std::move(packing), 1, 0.1, 1234, std::move(volumeScaler), {2, 2, 1});
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(1, 1, 10000, 15000, 1000, 1000, sphereTraits, std::move(collector), {}, logger);

    Quantity density = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;

    double expected = 0.398574;

    {
        INFO("Carnahan-Starling density: " << expected);
        INFO("Monte Carlo density: " << density);
        CHECK(density.value == Approx(expected).margin(density.error * 3)); // 3 sigma tolerance
        CHECK(density.error / density.value < 0.03); // up to 3%
    }

    // Second part - check if runs are deterministic w.r.t. to a seed

    auto shapes2 = OrthorhombicArrangingModel{}.arrange(200, dimensions);
    auto pbc2 = std::make_unique<PeriodicBoundaryConditions>();
    auto packing2 = std::make_unique<Packing>(
        dimensions, std::move(shapes2), std::move(pbc2), sphereTraits.getInteraction(), sphereTraits.getDataManager(),
        4, 4
    );
    auto volumeScaler2 = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>(), 1);
    Simulation simulation2(std::move(packing2), 1, 0.1, 1234, std::move(volumeScaler2), {2, 2, 1});
    auto collector2 = std::make_unique<ObservablesCollector>();
    collector2->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);

    simulation2.integrate(1, 1, 10000, 15000, 1000, 1000, sphereTraits, std::move(collector2), {}, logger);

    Quantity density2 = simulation2.getObservablesCollector().getFlattenedAverageValues().front().quantity;
    CHECK(density.value == density2.value);
    CHECK(density.error == density2.error);
}

TEST_CASE("Simulation: domain number auto-reduction", "[medium]") {
    OMP_SET_NUM_THREADS(4);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    double V = 250;
    double linearSize = std::cbrt(V);
    std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
    auto shapes = OrthorhombicArrangingModel{}.arrange(50, dimensions);
    SphereTraits sphereTraits(0.5);
    auto packing = std::make_unique<Packing>(
        dimensions, std::move(shapes), std::move(pbc), sphereTraits.getInteraction(), sphereTraits.getDataManager(), 4, 4
    );
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>(), 1);
    Simulation simulation(std::move(packing), 1, 0.1, 1234, std::move(volumeScaler), {4, 1, 1});
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NumberDensity>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(1, 1, 10000, 15000, 1000, 1000, sphereTraits, std::move(collector), {}, logger);

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
        OMP_SET_NUM_THREADS(1);
        auto pbc = std::make_unique<PeriodicBoundaryConditions>();
        double linearSize = 5;
        std::array<double, 3> dimensions = {linearSize, linearSize, linearSize};
        auto shapes = OrthorhombicArrangingModel{}.arrange(216, dimensions);    // 6 x 6 x 6

        auto sphereTraits = std::make_unique<SphereTraits>(0.5);
        auto squareInverseCore = std::make_unique<SquareInverseCoreInteraction>(1, 1);
        auto helperSphereTraits = std::make_unique<SphereTraits>(0.5, std::move(squareInverseCore));
        CompoundShapeTraits compoundSphere(std::move(sphereTraits), std::move(helperSphereTraits));

        auto packing = std::make_unique<Packing>(
            dimensions, std::move(shapes), std::move(pbc), compoundSphere.getInteraction(),
            compoundSphere.getDataManager(), numDomains, numDomains
        );
        auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>(), 1);
        Simulation simulation(std::move(packing), 0.1, 0.1, 1234, std::move(volumeScaler), domainDivisions);
        auto collector = std::make_unique<ObservablesCollector>();
        std::ostringstream loggerStream;
        Logger logger(loggerStream);

        simulation.relaxOverlaps(1, 5, 1000, compoundSphere, std::move(collector), {}, logger);

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

TEST_CASE("Simulation: upscaling skip stress test", "[short]") {
    OMP_SET_NUM_THREADS(1);

    auto cell = UnitCellFactory::createFccCell(1.43);
    Lattice lattice(cell, {3, 3, 3});
    auto box = lattice.getLatticeBox();
    auto shapes = lattice.generateMolecules();
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    SphereTraits sphereTraits(0.5);
    auto packing = std::make_unique<Packing>(
        box, std::move(shapes), std::move(pbc), sphereTraits.getInteraction(), sphereTraits.getDataManager()
    );
    auto volumeScaler = std::make_unique<TriclinicDeltaScaler>(0.0005, true);
    std::vector<std::unique_ptr<MoveSampler>> moveSamplers;
    moveSamplers.push_back(std::make_unique<TranslationSampler>(0.001));
    Simulation simulation(std::move(packing), std::move(moveSamplers), 1234, std::move(volumeScaler));
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<OverlapGuard>(), ObservablesCollector::SNAPSHOT);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(1, 100, 9000, 1000, 100, 1, sphereTraits, std::move(collector), {}, logger);
}

TEST_CASE("Simulation: hard dumbbell NVT relaxation", "[short]") {
    OMP_SET_NUM_THREADS(1);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    Lattice lattice(UnitCellFactory::createScCell({7.2/6, 7.2/6, 7.2/3}), {6, 6, 3});
    auto shapes = lattice.generateMolecules();
    KMerTraits kmerTraits(2, 0.5, 1);
    auto packing = std::make_unique<Packing>(
        lattice.getLatticeBox(), std::move(shapes), std::move(pbc), kmerTraits.getInteraction(),
        kmerTraits.getDataManager()
    );
    // More frequent averaging here to preserve short simulation times (particle displacement are large anyway)
    Simulation simulation(std::move(packing), 0.3, 0.3, 1234, nullptr);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<NematicOrder>(), ObservablesCollector::AVERAGING);

    simulation.integrate(1, 2, 5000, 5000, 100, 100, kmerTraits, std::move(collector), {}, logger);

    Quantity P2 = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;
    CHECK(std::abs(P2.value) < 0.05);
    CHECK(simulation.getPacking().getNumberDensity() == Approx(108/7.2/7.2/7.2));
}

TEST_CASE("Simulation: binary hard sphere gas", "[medium]") {
    OMP_SET_NUM_THREADS(1);

    // 125 spheres, out of which 8 have radius 0.5 and the rest - 0.1
    using SphereData = SphereTraits::HardData;
    ShapeData smallSphere(SphereData{0.1});
    ShapeData largeSphere(SphereData{0.5});
    Lattice lattice(UnitCellFactory::createScCell(1.1, smallSphere), {5, 5, 5});
    for (std::size_t i : {1, 3})
        for (std::size_t j : {1, 3})
            for (std::size_t k : {1, 3})
                lattice.modifySpecificCellMolecules(i, j, k)[0].setData(largeSphere);
    auto shapes = lattice.generateMolecules();
    auto box = lattice.getLatticeBox();

    SphereTraits sphereTraits(0.5);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    auto packing = std::make_unique<Packing>(
        box, std::move(shapes), std::move(pbc), sphereTraits.getInteraction(), sphereTraits.getDataManager()
    );
    auto volumeScaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>(), 1);
    Simulation simulation(std::move(packing), 1, 0.1, 1234, std::move(volumeScaler));
    auto collector = std::make_unique<ObservablesCollector>();
    collector->addObservable(std::make_unique<PackingFraction>(), ObservablesCollector::AVERAGING);
    std::ostringstream loggerStream;
    Logger logger(loggerStream);

    simulation.integrate(1, 7, 3000, 7000, 200, 200, sphereTraits, std::move(collector), {}, logger);

    Quantity actual = simulation.getObservablesCollector().getFlattenedAverageValues().front().quantity;
    double expected = 0.17294;  // Enciso et al. (1997). Mol. Phys., 92(2), 173-176
    INFO("Enciso eta  : " << expected);
    INFO("RAMPACK eta : " << actual);
    CHECK(actual.value == Approx(expected).margin(actual.error * 3)); // 3 sigma tolerance
    CHECK(actual.error / actual.value < 0.03); // up to 3%
}