//
// Created by pkua on 05.04.2022.
//

#include <catch2/catch.hpp>

#include "matchers/MatrixApproxMatcher.h"
#include "matchers/VectorApproxMatcher.h"

#include "core/Simulation.h"
#include "core/io/RamtrjRecorder.h"
#include "core/io/RamtrjPlayer.h"
#include "core/shapes/KMerTraits.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/volume_scalers/DeltaVolumeScaler.h"
#include "core/volume_scalers/TriclinicAdapter.h"
#include "core/lattice/legacy/OrthorhombicArrangingModel.h"
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

TEST_CASE("Simulation IO: storing and restoring") {
    KMerTraits traits(2, 0.5, 1);
    const auto &interaction = traits.getInteraction();
    const auto &dataManager = traits.getDataManager();
    legacy::OrthorhombicArrangingModel arrangingModel;
    auto shapes = arrangingModel.arrange(64, {10, 10, 10});
    TriclinicBox box(10);
    std::ostringstream logger_stream;
    Logger logger(logger_stream);
    std::stringbuf inout_buf;

    // Player packing
    auto pbc1 = std::make_unique<PeriodicBoundaryConditions>();
    Packing packing1(box, shapes, std::move(pbc1), interaction, dataManager, 1, 1);

    // Simulation and recorder packing
    auto pbc2 = std::make_unique<PeriodicBoundaryConditions>();
    auto packing2 = std::make_unique<Packing>(box, shapes, std::move(pbc2), interaction, dataManager, 1, 1);
    std::vector<std::unique_ptr<MoveSampler>> moveSamplers;
    moveSamplers.push_back(std::make_unique<RototranslationSampler>(0.5, 0.1));
    auto scaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>(), 1);
    Simulation simulation(std::move(packing2), std::move(moveSamplers), 1234, std::move(scaler));

    SECTION("without continuation") {
        auto inout_stream = std::make_unique<std::iostream>(&inout_buf);
        std::vector<std::unique_ptr<SimulationRecorder>> recorders;
        recorders.push_back(std::make_unique<RamtrjRecorder>(std::move(inout_stream), simulation.getPacking().size(),
                                                             100, false));
        auto collector = std::make_unique<ObservablesCollector>();
        simulation.integrate(1, 1, 1000, 1000, 100, 100, traits, std::move(collector), std::move(recorders), logger);

        auto in_stream = std::make_unique<std::istream>(&inout_buf);
        // We also check is auto fix will correctly tell that no fixing in needed
        RamtrjPlayer::AutoFix autoFix(simulation.getPacking().size());
        RamtrjPlayer player(std::move(in_stream), autoFix);

        CHECK_FALSE(autoFix.wasFixingNeeded());
        CHECK(player.getTotalCycles() == 2000);
        CHECK(player.getCycleStep() == 100);

        SECTION("whole replay") {
            while (player.hasNext())
                player.nextSnapshot(packing1, interaction, dataManager);
            player.close();

            assert_equal(packing1, simulation.getPacking());
        }

        SECTION("only last") {
            player.lastSnapshot(packing1, interaction, dataManager);
            player.close();

            assert_equal(packing1, simulation.getPacking());
        }

        SECTION("only last, but using explicit cycle number") {
            player.jumpToSnapshot(packing1, interaction, dataManager, 2000);
            player.close();

            assert_equal(packing1, simulation.getPacking());
        }
    }

    SECTION("with continuation") {
        // Initial run
        auto inout_stream1 = std::make_unique<std::iostream>(&inout_buf);
        std::vector<std::unique_ptr<SimulationRecorder>> recorders1;
        recorders1.push_back(std::make_unique<RamtrjRecorder>(std::move(inout_stream1), simulation.getPacking().size(),
                                                              100, false));
        auto collector1 = std::make_unique<ObservablesCollector>();
        simulation.integrate(1, 1, 500, 500, 100, 100, traits, std::move(collector1), std::move(recorders1), logger);

        // Continuation
        auto inout_stream2 = std::make_unique<std::iostream>(&inout_buf);
        std::vector<std::unique_ptr<SimulationRecorder>> recorders2;
        recorders2.push_back(std::make_unique<RamtrjRecorder>(std::move(inout_stream2), simulation.getPacking().size(),
                                                              100, true));
        CHECK(recorders2.front()->getLastCycleNumber() == 1000);
        auto collector2 = std::make_unique<ObservablesCollector>();
        simulation.integrate(1, 1, 500, 500, 100, 100, traits, std::move(collector2), std::move(recorders2), logger,
                             1000);

        auto in_stream = std::make_unique<std::istream>(&inout_buf);
        RamtrjPlayer player(std::move(in_stream));
        CHECK(player.getTotalCycles() == 2000);
        CHECK(player.getCycleStep() == 100);
        while (player.hasNext())
            player.nextSnapshot(packing1, interaction, dataManager);
        player.close();

        assert_equal(packing1, simulation.getPacking());
    }

    SECTION("with continuation from 0 snapshots") {
        // Initial run
        auto inout_stream1 = std::make_unique<std::iostream>(&inout_buf);
        auto recorder1 = std::make_unique<RamtrjRecorder>(std::move(inout_stream1), simulation.getPacking().size(),
                                                          100, false);
        recorder1.reset();

        // Continuation
        auto inout_stream2 = std::make_unique<std::iostream>(&inout_buf);
        std::vector<std::unique_ptr<SimulationRecorder>> recorders2;
        recorders2.push_back(std::make_unique<RamtrjRecorder>(std::move(inout_stream2), simulation.getPacking().size(),
                                                              100, true));
        CHECK(recorders2.front()->getLastCycleNumber() == 0);
        auto collector2 = std::make_unique<ObservablesCollector>();
        simulation.integrate(1, 1, 500, 500, 100, 100, traits, std::move(collector2), std::move(recorders2), logger);

        auto in_stream = std::make_unique<std::istream>(&inout_buf);
        RamtrjPlayer player(std::move(in_stream));
        CHECK(player.getTotalCycles() == 1000);
        CHECK(player.getCycleStep() == 100);
        while (player.hasNext())
            player.nextSnapshot(packing1, interaction, dataManager);
        player.close();

        assert_equal(packing1, simulation.getPacking());
    }

    SECTION("fixing trajectory") {
        auto inout_stream = std::make_unique<std::iostream>(&inout_buf);
        std::vector<std::unique_ptr<SimulationRecorder>> recorders;
        recorders.push_back(std::make_unique<RamtrjRecorder>(std::move(inout_stream), simulation.getPacking().size(),
                                                             100, false));
        auto collector = std::make_unique<ObservablesCollector>();
        simulation.integrate(1, 1, 1000, 1000, 100, 100, traits, std::move(collector), std::move(recorders), logger);

        // Add garbage bytes
        {
            std::iostream inout(&inout_buf);
            inout.seekp(0, std::ios::end);
            inout.write("12345", 5);
        }

        auto in_stream = std::make_unique<std::istream>(&inout_buf);
        RamtrjPlayer::AutoFix autoFix(simulation.getPacking().size());
        RamtrjPlayer player(std::move(in_stream), autoFix);

        CHECK(autoFix.wasFixingNeeded());
        CHECK(autoFix.wasFixingSuccessful());
        CHECK(autoFix.getInferredSnapshots() == 20);
        CHECK(autoFix.getBytesRemainder() == 5);
        CHECK(player.getTotalCycles() == 2000);
        CHECK(player.getCycleStep() == 100);

        while (player.hasNext())
            player.nextSnapshot(packing1, interaction, dataManager);
        player.close();

        assert_equal(packing1, simulation.getPacking());
    }
}