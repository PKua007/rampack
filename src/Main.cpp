//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cstdlib>
#include <string>
#include <iostream>

#include "frontend/Frontend.h"
#include "utils/Logger.h"

int main(int argc, char **argv) {
    Logger logger(std::cout);

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

    Frontend frontend(logger);
    if (mode == "-h" || mode == "--help")
        return frontend.printGeneralHelp(cmd);
    else if (mode == "casino")
        return frontend.casino(argc, argv);
    else if (mode == "analyze")
        return frontend.analyze(argc, argv);
    else if (mode == "optimize-distance")
        return frontend.optimize_distance(argc, argv);

    logger.error() << "Unknown mode " << mode << std::endl;
    return EXIT_FAILURE;
}