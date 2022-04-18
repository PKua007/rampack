//
// Created by Piotr Kubala on 08/09/2020.
//

#include <sstream>

#include <catch2/catch.hpp>

#include "utils/Logger.h"

namespace {
    class LoggerUnderTest : public Logger {
    protected:
        [[nodiscard]] std::string currentDateTime() const override { return "date"; }

    public:
        explicit LoggerUnderTest(std::ostream &out) : Logger(out) { }
    };
}

TEST_CASE("Logger: types") {
    std::ostringstream out;
    LoggerUnderTest logger(out);
    logger.setVerbosityLevel(Logger::DEBUG);

    logger.error() << "error" << std::endl;
    logger.warn() << "warn" << std::endl;
    logger.info() << "info" << std::endl;
    logger.verbose() << "verbose" << std::endl;
    logger.debug() << "debug" << std::endl;

    REQUIRE(out.str() == "[  ERROR] [date] error\n"
                         "[   WARN] [date] warn\n"
                         "[   INFO] [date] info\n"
                         "[VERBOSE] [date] verbose\n"
                         "[  DEBUG] [date] debug\n");
}

TEST_CASE("Logger: behaviour") {
    std::ostringstream out;
    LoggerUnderTest logger(out);

    logger << "info should be default" << std::endl;
    logger.info() << "flush " << std::flush;
    logger.info() << "test" << std::endl;
    logger.info() << "change of type in the middle";
    logger.warn() << "should make a newline" << std::endl;
    logger.info() << "2 + 2 = " << (2 + 2) << std::endl;

    REQUIRE(out.str() == "[   INFO] [date] info should be default\n"
                         "[   INFO] [date] flush test\n"
                         "[   INFO] [date] change of type in the middle\n"
                         "[   WARN] [date] should make a newline\n"
                         "[   INFO] [date] 2 + 2 = 4\n");
}

TEST_CASE("Logger: additional text") {
    std::ostringstream out;
    LoggerUnderTest logger(out);

    SECTION("default additional text should be empty") {
        REQUIRE(logger.getAdditionalText().empty());
    }

    SECTION("displaying additional text") {
        logger.setAdditionalText("additional text");
        logger << "info" << std::endl;

        REQUIRE(logger.getAdditionalText() == "additional text");
        REQUIRE(out.str() == "[   INFO] [date] [additional text] info\n");
    }
}

TEST_CASE("Logger: verbosity level") {
    std::ostringstream out;
    LoggerUnderTest logger(out);
    auto verbosity = GENERATE(std::make_pair("[  ERROR]", Logger::ERROR),
                              std::make_pair("[   WARN]", Logger::WARN),
                              std::make_pair("[   INFO]", Logger::INFO),
                              std::make_pair("[VERBOSE]", Logger::VERBOSE),
                              std::make_pair("[  DEBUG]", Logger::DEBUG));

    logger.setVerbosityLevel(verbosity.second);
    logger.debug() << "debug" << std::endl;
    logger.verbose() << "verbose" << std::endl;
    logger.info() << "info" << std::endl;
    logger.warn() << "warn" << std::endl;
    logger.error() << "error" << std::endl;

    REQUIRE_THAT(out.str(), Catch::StartsWith(verbosity.first));
}

TEST_CASE("Logger: multiple outputs") {
    std::ostringstream out1;
    std::ostringstream out2;
    LoggerUnderTest logger(out1);

    SECTION("copying verbosity") {
        logger.setVerbosityLevel(Logger::WARN);
        logger.addOutput(out2);
        logger.warn() << "warn" << std::endl;
        logger.info() << "info" << std::endl;

        CHECK(out1.str() == "[   WARN] [date] warn\n");
        CHECK(out2.str() == "[   WARN] [date] warn\n");

        SECTION("removing output") {
            logger.removeOutput(out1);
            logger.warn() << "warn2" << std::endl;

            CHECK(out1.str() == "[   WARN] [date] warn\n");
            CHECK(out2.str() == "[   WARN] [date] warn\n[   WARN] [date] warn2\n");

            SECTION("cannot remove the last one") {
                CHECK_THROWS(logger.removeOutput(out2));
            }
        }
    }

    SECTION("setting verbosity for all") {
        logger.setVerbosityLevel(Logger::ERROR);
        logger.addOutput(out2);
        logger.setVerbosityLevel(Logger::WARN);
        logger.warn() << "warn" << std::endl;
        logger.info() << "info" << std::endl;

        CHECK(out1.str() == "[   WARN] [date] warn\n");
        CHECK(out2.str() == "[   WARN] [date] warn\n");
    }

    SECTION("setting verbosity for individual") {
        logger.addOutput(out2);
        logger.setVerbosityLevel(Logger::INFO, out1);
        logger.setVerbosityLevel(Logger::WARN, out2);
        logger.warn() << "warn" << std::endl;
        logger.info() << "info" << std::endl;

        CHECK(out1.str() == "[   WARN] [date] warn\n[   INFO] [date] info\n");
        CHECK(out2.str() == "[   WARN] [date] warn\n");
    }
}