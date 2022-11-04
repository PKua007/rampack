//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cstdlib>
#include <string>
#include <iostream>

#include "frontend/Frontend.h"
#include "utils/Logger.h"


namespace {
    Logger logger(std::cout);

    void logger_terminate_handler() {
        logger.error() << "Terminate called after throwing an instance of ";
        try {
            std::rethrow_exception(std::current_exception());
        } catch (const std::exception &ex) {
            logger << typeid(ex).name() << std::endl;
            logger << "what(): " << ex.what() << std::endl;
        }
        std::abort();
    }

    int handle_commands(const std::string &cmd, const std::string &mode, int argc, char **argv) {
        Frontend frontend(logger);
        if (mode == "-h" || mode == "--help")
            return frontend.printGeneralHelp(cmd);
        else if (mode == "casino")
            return frontend.casino(argc, argv);
        else if (mode == "optimize-distance")
            return frontend.optimize_distance(argc, argv);
        else if (mode == "preview")
            return frontend.preview(argc, argv);
        else if (mode == "trajectory")
            return frontend.trajectory(argc, argv);
        logger.error() << "Unknown mode " << mode << ". See " << cmd << " --help" << std::endl;
        return EXIT_FAILURE;
    }
}


int main(int argc, char **argv) {
    std::set_terminate(logger_terminate_handler);

    std::string cmd(argv[0]);
    if (argc < 2) {
        logger.error() << "Usage: " << cmd << " [mode] (mode dependent parameters). " << std::endl;
        logger << "Type " << cmd << " --help to see available modes" << std::endl;
        exit(EXIT_FAILURE);
    }

    // We now shift the arguments, and pretend, that the new first (command) is "cmd mode"
    // Different modes can then parse the arguments separately and will think that the whole argv[0] is "cmd mode"
    std::string mode(argv[1]);
    std::string cmdAndMode = cmd + " " + mode;
    argv++;
    argc--;
    argv[0] = cmdAndMode.data();


    // Try and rethrow exception to let the destructors be called during stack unwinding, but then use the general
    // terminate handler. This however does not work for exceptions thrown from threads other than master.
    // This is enabled for the Release build - in the Debug build we want to know from where the exception is thrown
    // and using rethrowing erases this information, and we do not need destructors to be called, since the execution
    // is only paused, so we have access to everything anyway.
    #ifdef NDEBUG
        try {
            handle_commands(cmd, mode, argc, argv);
        } catch (const std::exception &) {
            throw;
        }
    #else
        handle_commands(cmd, mode, argc, argv);
    #endif
}