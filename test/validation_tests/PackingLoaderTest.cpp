//
// Created by Piotr Kubala on 28/06/2023.
//

#include <catch2/catch.hpp>
#include <filesystem>
#include <sstream>

#include "frontend/PackingLoader.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/shapes/SphereTraits.h"


#define SOURCE_DIR std::filesystem::path(__FILE__).remove_filename()


TEST_CASE("PackingLoader") {
    const auto PACKING_FINISHED_PATH = SOURCE_DIR / "data/packing_loader/packing_finished.ramsnap";
    const auto PACKING_UNFINISHED_PATH = SOURCE_DIR / "data/packing_loader/packing_unfinished.ramsnap";
    std::ostringstream loggerStream;
    Logger logger(loggerStream);
    auto bc = std::make_unique<PeriodicBoundaryConditions>();
    SphereTraits sphereTraits(0.5);
    const auto &interaction = sphereTraits.getInteraction();

    SECTION("finished + finished") {
        IntegrationRun run1;
        run1.runName = "run1";
        run1.thermalizationCycles = 10000;
        run1.averagingCycles = 10000;
        run1.ramsnapOut = PACKING_FINISHED_PATH;
        IntegrationRun run2;
        run2.runName = "run2";
        run2.thermalizationCycles = 10000;
        run2.averagingCycles = 10000;
        run2.ramsnapOut = PACKING_FINISHED_PATH;
        std::vector<Run> runs = {run1, run2};

        SECTION("start from scratch") {
            PackingLoader loader(logger, std::nullopt, std::nullopt, runs);

            loader.loadPacking(std::move(bc), interaction, 1, 1);

            CHECK_FALSE(loader.isContinuation());
            CHECK_FALSE(loader.isAllFinished());
            CHECK_FALSE(loader.isRestored());
            CHECK(loader.getCycleOffset() == 0);
            CHECK(loader.getStartRunIndex() == 0);
        }

        SECTION("start from second") {
            PackingLoader loader(logger, "run2", std::nullopt, runs);

            loader.loadPacking(std::move(bc), interaction, 1, 1);

            CHECK_FALSE(loader.isContinuation());
            CHECK_FALSE(loader.isAllFinished());
            CHECK(loader.isRestored());
            CHECK(loader.getCycleOffset() == 0);
            CHECK(loader.getStartRunIndex() == 1);
        }

        SECTION("continue first") {
            PackingLoader loader(logger, std::nullopt, 30000, runs);

            loader.loadPacking(std::move(bc), interaction, 1, 1);

            CHECK(loader.isContinuation());
            CHECK_FALSE(loader.isAllFinished());
            CHECK(loader.isRestored());
            CHECK(loader.getCycleOffset() == 20000);
            CHECK(loader.getStartRunIndex() == 0);
        }

        SECTION("continue second") {
            PackingLoader loader(logger, "run2", 30000, runs);

            loader.loadPacking(std::move(bc), interaction, 1, 1);

            CHECK(loader.isContinuation());
            CHECK_FALSE(loader.isAllFinished());
            CHECK(loader.isRestored());
            CHECK(loader.getCycleOffset() == 20000);
            CHECK(loader.getStartRunIndex() == 1);
        }

        SECTION("continue first (thermalization finished + no averaging)") {
            run1.averagingCycles = 0;
            runs = {run1, run2};
            PackingLoader loader(logger, std::nullopt, 20000, runs);

            loader.loadPacking(std::move(bc), interaction, 1, 1);

            CHECK_FALSE(loader.isContinuation());
            CHECK_FALSE(loader.isAllFinished());
            CHECK(loader.isRestored());
            CHECK(loader.getCycleOffset() == 0);
            CHECK(loader.getStartRunIndex() == 1);
        }

        SECTION("continue second (thermalization finished + no averaging)") {
            run2.averagingCycles = 0;
            runs = {run1, run2};
            PackingLoader loader(logger, "run2", 20000, runs);

            loader.loadPacking(std::move(bc), interaction, 1, 1);

            CHECK_FALSE(loader.isContinuation());
            CHECK(loader.isAllFinished());
            CHECK(loader.isRestored());
            CHECK(loader.getCycleOffset() == 0);
            CHECK(loader.getStartRunIndex() == 2);
        }

        SECTION(".auto - all finished") {
            PackingLoader loader(logger, ".auto", std::nullopt, runs);

            loader.loadPacking(std::move(bc), interaction, 1, 1);

            CHECK_FALSE(loader.isContinuation());
            CHECK(loader.isAllFinished());
            CHECK_FALSE(loader.isRestored());
            CHECK(loader.getCycleOffset() == 0);
            CHECK(loader.getStartRunIndex() == 2);
        }
    }

    SECTION("finished + unfinished") {
        IntegrationRun run1;
        run1.runName = "run1";
        run1.thermalizationCycles = 10000;
        run1.averagingCycles = 10000;
        run1.ramsnapOut = PACKING_FINISHED_PATH;
        IntegrationRun run2;
        run2.runName = "run2";
        run2.thermalizationCycles = 10000;
        run2.averagingCycles = 10000;
        run2.ramsnapOut = PACKING_UNFINISHED_PATH;
        std::vector<Run> runs = {run1, run2};

        SECTION(".auto - continue second") {
            PackingLoader loader(logger, ".auto", std::nullopt, runs);

            loader.loadPacking(std::move(bc), interaction, 1, 1);

            CHECK(loader.isContinuation());
            CHECK_FALSE(loader.isAllFinished());
            CHECK(loader.isRestored());
            CHECK(loader.getCycleOffset() == 3000);
            CHECK(loader.getStartRunIndex() == 1);
        }
    }

    SECTION("finished + unfinished + finished") {
        IntegrationRun run1;
        run1.runName = "run1";
        run1.thermalizationCycles = 10000;
        run1.averagingCycles = 10000;
        run1.ramsnapOut = PACKING_FINISHED_PATH;
        IntegrationRun run2;
        run2.runName = "run2";
        run2.thermalizationCycles = 10000;
        run2.averagingCycles = 10000;
        run2.ramsnapOut = PACKING_UNFINISHED_PATH;
        IntegrationRun run3;
        run3.runName = "run3";
        run3.thermalizationCycles = 10000;
        run3.averagingCycles = 10000;
        run3.ramsnapOut = PACKING_FINISHED_PATH;
        std::vector<Run> runs = {run1, run2, run3};

        SECTION(".auto - error") {
            PackingLoader loader(logger, ".auto", std::nullopt, runs);

            CHECK_THROWS_WITH(
                loader.loadPacking(std::move(bc), interaction, 1, 1), Catch::Contains("detected already performed run")
            );
        }
    }

    SECTION("finished + none") {
        IntegrationRun run1;
        run1.runName = "run1";
        run1.thermalizationCycles = 10000;
        run1.averagingCycles = 10000;
        run1.ramsnapOut = PACKING_FINISHED_PATH;
        IntegrationRun run2;
        run2.runName = "run2";
        run2.thermalizationCycles = 10000;
        run2.averagingCycles = 10000;
        run2.ramsnapOut = "/non/existent/path";
        std::vector<Run> runs = {run1, run2};

        SECTION(".auto - start second from scratch") {
            PackingLoader loader(logger, ".auto", std::nullopt, runs);

            loader.loadPacking(std::move(bc), interaction, 1, 1);

            CHECK_FALSE(loader.isContinuation());
            CHECK_FALSE(loader.isAllFinished());
            CHECK(loader.isRestored());
            CHECK(loader.getCycleOffset() == 0);
            CHECK(loader.getStartRunIndex() == 1);
        }
    }
}