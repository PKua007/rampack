//
// Created by pkua on 05.04.2022.
//

#include <catch2/catch.hpp>
#include <filesystem>
#include <fstream>

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


#define SOURCE_DIR std::filesystem::path(__FILE__).remove_filename()

namespace {
    void assert_equal(const Packing &actualPacking, const Packing &expectedPacking) {
        const auto &box1 = actualPacking.getBox();
        const auto &box2 = expectedPacking.getBox();
        CHECK_THAT(box1.getDimensions(), IsApproxEqual(box2.getDimensions(), 1e-12));

        REQUIRE(actualPacking.size() == expectedPacking.size());
        for (std::size_t i{}; i < actualPacking.size(); i++) {
            const auto &shape1 = actualPacking[i];
            const auto &shape2 = expectedPacking[i];
            CHECK_THAT(shape1.getPosition(), IsApproxEqual(shape2.getPosition(), 1e-12));
            CHECK_THAT(shape1.getOrientation(), IsApproxEqual(shape2.getOrientation(), 1e-12));
            CHECK(shape1.getData() == shape2.getData());
        }
    }

    std::unique_ptr<BoundaryConditions> create_pbc() {
        return std::make_unique<PeriodicBoundaryConditions>();
    }

    void read_buf(const std::istream &in, std::streambuf &out) {
        std::ostream(&out) << in.rdbuf();
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
    Packing packing1(box, shapes, create_pbc(), interaction, dataManager, 1, 1);

    // Simulation and recorder packing
    auto packing2 = std::make_unique<Packing>(box, shapes, create_pbc(), interaction, dataManager, 1, 1);
    std::vector<std::unique_ptr<MoveSampler>> moveSamplers;
    moveSamplers.push_back(std::make_unique<RototranslationSampler>(0.5, 0.1));
    auto scaler = std::make_unique<TriclinicAdapter>(std::make_unique<DeltaVolumeScaler>(), 1);
    Simulation simulation(std::move(packing2), std::move(moveSamplers), 1234, std::move(scaler));

    SECTION("without continuation") {
        // Recording
        auto inout_stream = std::make_unique<std::iostream>(&inout_buf);
        std::vector<std::unique_ptr<SimulationRecorder>> recorders;
        recorders.push_back(std::make_unique<RamtrjRecorder>(std::move(inout_stream), simulation.getPacking(),
                                                             traits.getDataManager(), 100, false));
        auto collector = std::make_unique<ObservablesCollector>();
        simulation.integrate(1, 1, 1000, 1000, 100, 100, traits, std::move(collector), std::move(recorders), logger);

        // Replay
        auto in_stream = std::make_unique<std::istream>(&inout_buf);
        // We also check is auto fix will correctly tell that no fixing in needed
        RamtrjPlayer::AutoFix autoFix(simulation.getPacking().size());
        RamtrjPlayer player(std::move(in_stream), traits.getDataManager(), autoFix);

        CHECK_FALSE(autoFix.wasFixingNeeded());
        CHECK(player.getTotalCycles() == 2000);
        CHECK(player.getCycleStep() == 100);

        SECTION("whole replay") {
            while (player.hasNext())
                player.nextSnapshot(packing1, traits);
            player.close();

            assert_equal(packing1, simulation.getPacking());
        }

        SECTION("only last") {
            player.lastSnapshot(packing1, traits);
            player.close();

            assert_equal(packing1, simulation.getPacking());
        }

        SECTION("only last, but using explicit cycle number") {
            player.jumpToSnapshot(packing1, traits, 2000);
            player.close();

            assert_equal(packing1, simulation.getPacking());
        }

        SECTION("new number of particles") {
            shapes.push_back(Shape({5, 5, 5}));     // one more shape in the packing before replaying snapshot
            packing1.reset(shapes, packing1.getBox(), interaction, dataManager);

            player.lastSnapshot(packing1, traits);

            assert_equal(packing1, simulation.getPacking());
        }
    }

    SECTION("with continuation") {
        // Initial run
        auto inout_stream1 = std::make_unique<std::iostream>(&inout_buf);
        std::vector<std::unique_ptr<SimulationRecorder>> recorders1;
        recorders1.push_back(std::make_unique<RamtrjRecorder>(std::move(inout_stream1), simulation.getPacking(),
                                                              traits.getDataManager(), 100, false));
        auto collector1 = std::make_unique<ObservablesCollector>();
        simulation.integrate(1, 1, 500, 500, 100, 100, traits, std::move(collector1), std::move(recorders1), logger);

        // Continuation
        auto inout_stream2 = std::make_unique<std::iostream>(&inout_buf);
        std::vector<std::unique_ptr<SimulationRecorder>> recorders2;
        recorders2.push_back(std::make_unique<RamtrjRecorder>(std::move(inout_stream2), simulation.getPacking(),
                                                              traits.getDataManager(), 100, true));
        CHECK(recorders2.front()->getLastCycleNumber() == 1000);
        auto collector2 = std::make_unique<ObservablesCollector>();
        simulation.integrate(1, 1, 500, 500, 100, 100, traits, std::move(collector2), std::move(recorders2), logger,
                             1000);

        // Replay
        auto in_stream = std::make_unique<std::istream>(&inout_buf);
        RamtrjPlayer player(std::move(in_stream), traits.getDataManager());
        CHECK(player.getTotalCycles() == 2000);
        CHECK(player.getCycleStep() == 100);
        while (player.hasNext())
            player.nextSnapshot(packing1, traits);
        player.close();

        assert_equal(packing1, simulation.getPacking());
    }

    SECTION("with continuation from 0 snapshots") {
        // Initial run
        auto inout_stream1 = std::make_unique<std::iostream>(&inout_buf);
        auto recorder1 = std::make_unique<RamtrjRecorder>(std::move(inout_stream1), simulation.getPacking(),
                                                          traits.getDataManager(), 100, false);
        recorder1.reset();

        // Continuation
        auto inout_stream2 = std::make_unique<std::iostream>(&inout_buf);
        std::vector<std::unique_ptr<SimulationRecorder>> recorders2;
        recorders2.push_back(std::make_unique<RamtrjRecorder>(std::move(inout_stream2), simulation.getPacking(),
                                                              traits.getDataManager(), 100, true));
        CHECK(recorders2.front()->getLastCycleNumber() == 0);
        auto collector2 = std::make_unique<ObservablesCollector>();
        simulation.integrate(1, 1, 500, 500, 100, 100, traits, std::move(collector2), std::move(recorders2), logger);

        // Replay
        auto in_stream = std::make_unique<std::istream>(&inout_buf);
        RamtrjPlayer player(std::move(in_stream), traits.getDataManager());
        CHECK(player.getTotalCycles() == 1000);
        CHECK(player.getCycleStep() == 100);
        while (player.hasNext())
            player.nextSnapshot(packing1, traits);
        player.close();

        assert_equal(packing1, simulation.getPacking());
    }

    SECTION("fixing trajectory") {
        auto inout_stream = std::make_unique<std::iostream>(&inout_buf);
        std::vector<std::unique_ptr<SimulationRecorder>> recorders;
        recorders.push_back(std::make_unique<RamtrjRecorder>(std::move(inout_stream), simulation.getPacking(),
                                                             traits.getDataManager(), 100, false));
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
        RamtrjPlayer player(std::move(in_stream), traits.getDataManager(), autoFix);

        CHECK(autoFix.wasFixingNeeded());
        CHECK(autoFix.wasFixingSuccessful());
        CHECK(autoFix.getInferredSnapshots() == 20);
        CHECK(autoFix.getBytesRemainder() == 5);
        CHECK(player.getTotalCycles() == 2000);
        CHECK(player.getCycleStep() == 100);

        while (player.hasNext())
            player.nextSnapshot(packing1, traits);
        player.close();

        assert_equal(packing1, simulation.getPacking());
    }
}

TEST_CASE("Simulation IO: backwards compatibility") {
    SECTION("v1.1") {
        KMerTraits traits(2, 0.5, 1);
        const auto &interaction = traits.getInteraction();
        const auto &dataManager = traits.getDataManager();
        std::ostringstream logger_stream;
        Logger logger(logger_stream);
        std::stringbuf inout_buf;

        // Packings
        std::vector<Shape> shapes1{Shape({1, 1, 1}, Matrix<3, 3>::identity()),
                                   Shape({2, 2, 2}, Matrix<3, 3>::rotation(M_PI / 4, 0, 0))};
        std::vector<Shape> shapes2{Shape({3, 3, 3}, Matrix<3, 3>::rotation(0, M_PI / 4, 0)),
                                   Shape({4, 4, 4}, Matrix<3, 3>::rotation(0, 0, M_PI / 4))};
        Packing snapshot1(TriclinicBox(3), shapes1, create_pbc(), interaction, dataManager);
        Packing snapshot2(TriclinicBox(5), shapes2, create_pbc(), interaction, dataManager);
        Packing playerPacking(TriclinicBox(3), shapes1, create_pbc(), interaction, dataManager);

        // Load initial v1.1 trajectory with one snapshot
        {
            const auto TRAJECTORY_V1P1_PATH = SOURCE_DIR / "data/ramtrj/trajectory_v1.1.ramtrj";
            std::ifstream initialTrajetoryIn(TRAJECTORY_V1P1_PATH, std::ios::in | std::ios::binary);
            read_buf(initialTrajetoryIn, inout_buf);
            REQUIRE_FALSE(initialTrajetoryIn.fail());
        }

        // Continuation (add another snapshot)
        {
            auto inout_stream2 = std::make_unique<std::iostream>(&inout_buf);
            RamtrjRecorder recorder(std::move(inout_stream2), snapshot2, traits.getDataManager(), 1000, true);
            CHECK(recorder.getLastCycleNumber() == 1000);
            recorder.recordSnapshot(snapshot2, traits, 2000);
        }

        // Replay
        {
            auto in_stream = std::make_unique<std::istream>(&inout_buf);
            RamtrjPlayer player(std::move(in_stream), traits.getDataManager());
            CHECK(player.getTotalCycles() == 2000);
            CHECK(player.getCycleStep() == 1000);

            REQUIRE(player.hasNext());
            player.nextSnapshot(playerPacking, traits);
            assert_equal(playerPacking, snapshot1);
            REQUIRE(player.hasNext());
            player.nextSnapshot(playerPacking, traits);
            assert_equal(playerPacking, snapshot2);
            CHECK_FALSE(player.hasNext());
        }
    }
}