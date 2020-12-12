//
// Created by Piotr Kubala on 12/12/2020.
//

#include <cstdlib>

#include "Frontend.h"
#include "utils/Fold.h"

int Frontend::casino(int argc, char **argv) {
    return EXIT_SUCCESS;
}

int Frontend::analyze(int argc, char **argv) {
    return EXIT_SUCCESS;
}

int Frontend::printGeneralHelp(const std::string &cmd) {
    std::ostream &rawOut = this->logger;

    rawOut << Fold("Random and Maximal PACKing PACKage - computational package dedicated to simulate various packing "
                   "models.").width(80) << std::endl;
    rawOut << std::endl;
    rawOut << "Usage: " << cmd << " [mode] (mode dependent parameters). " << std::endl;
    rawOut << std::endl;
    rawOut << "Available modules:" << std::endl;
    rawOut << "casino" << std::endl;
    rawOut << Fold("Hard particle Monte Carlo.").width(80).margin(4) << std::endl;
    rawOut << "analyze" << std::endl;
    rawOut << Fold("Statistical analysis of simulations.").width(80).margin(4) << std::endl;
    rawOut << std::endl;
    rawOut << "Type " + cmd + " [mode] --help to get help on the specific mode." << std::endl;

    return EXIT_SUCCESS;
}
