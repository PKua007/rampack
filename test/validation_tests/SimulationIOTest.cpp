//
// Created by pkua on 05.04.2022.
//

#include <catch2/catch.hpp>

#include "matchers/MatrixApproxMatcher.h"
#include "matchers/VectorApproxMatcher.h"

#include "core/Simulation.h"
#include "core/SimulationRecorder.h"
#include "core/SimulationPlayer.h"
#include "core/shapes/KMerTraits.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/volume_scalers/DeltaVolumeScaler.h"
#include "core/volume_scalers/TriclinicAdapter.h"
#include "core/arranging_models/OrthorhombicArrangingModel.h"
#include "core/move_samplers/RototranslationSampler.h"


namespace {
    void assert_equal(const Packing &packing1, const Packing &packing2) {
        const auto &box1 = packing1.getBox();
        const auto &box2 = packing2.getBox();
        CHECK_THAT(box1.getDimensions(), IsApproxEqual(box2.getDimensions(), 1e-12));

        REQUIRE(packing1.size() == packing2.size());
        for (std::size_t i{}; i < packing1.size(); i++) {
            const auto &shape1 = packing1[i];
            const auto &shape2 = packing2[i];
            CHECK_THAT(shape1.getPosition(), IsApproxEqual(shape2.getPosition(), 1e-12));
            CHECK_THAT(shape1.getOrientation(), IsApproxEqual(shape2.getOrientation(), 1e-12));
        }
    }
}

TEST_CASE("Simulation IO: storing and restoring")
{
    KMerTraits traits(2, 0.5, 1);
    const auto &interaction = traits.getInteraction();
    OrthorhombicArrangingModel arrangingModel;
    auto shapes = arrangingModel.arrange(64, {10, 10, 10});
    TriclinicBox box(10);
    std::ostringstream logger_stream;
    Logger logger(logger_stream);
    std::stringbuf inout_buf;

    // Player packing
    auto pbc1 = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing1(box, shapes, std::move(pbc1), interaction, 1, 1);

    // Simulation and recorder packing
    auto pbc2 = std::make_unique<PeriodicBoundaryConditions>();
    auto packing2 = std::make_unique<Packing>(box, shapes, std::move(pbc2), interaction, 1, 1);
    std::vector<std::unique_ptr<MoveSampler>> moveSamplers;
    moveSamplers.push_back(std::make_unique<RototranslationSampler>(0.5, 0.1));
    auto scaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>());
    Simulation simulation(std::move(packing2), std::move(moveSamplers), 1, 1234, std::move(scaler));

    SECTION("without continuation") {
        auto inout_stream = std::make_unique<std::iostream>(&inout_buf);
        auto recorder = std::make_unique<SimulationRecorder>(std::move(inout_stream), false);
        auto collector = std::make_unique<ObservablesCollector>();
        simulation.integrate(1, 1, 1000, 1000, 100, 100, traits, std::move(collector), std::move(recorder), logger);

        auto in_stream = std::make_unique<std::istream>(&inout_buf);
        SimulationPlayer player(std::move(in_stream));
        while (player.hasNext())
            player.nextSnapshot(packing1, interaction);
        player.close();

        assert_equal(packing1, simulation.getPacking());
    }

    SECTION("with continuation") {
        // Initial run
        auto inout_stream1 = std::make_unique<std::iostream>(&inout_buf);
        auto recorder1 = std::make_unique<SimulationRecorder>(std::move(inout_stream1), false);
        auto collector1 = std::make_unique<ObservablesCollector>();
        simulation.integrate(1, 1, 500, 500, 100, 100, traits, std::move(collector1), std::move(recorder1), logger);

        // Continuation
        auto inout_stream2 = std::make_unique<std::iostream>(&inout_buf);
        auto recorder2 = std::make_unique<SimulationRecorder>(std::move(inout_stream2), true);
        auto collector2 = std::make_unique<ObservablesCollector>();
        simulation.integrate(1, 1, 500, 500, 100, 100, traits, std::move(collector2), std::move(recorder2), logger,
                             1000);

        auto in_stream = std::make_unique<std::istream>(&inout_buf);
        SimulationPlayer player(std::move(in_stream));
        while (player.hasNext())
            player.nextSnapshot(packing1, interaction);
        player.close();

        assert_equal(packing1, simulation.getPacking());
    }
}