//
// Created by Piotr Kubala on 25/01/2023.
//

#include "HelpMode.h"
#include "utils/Fold.h"


int HelpMode::main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    std::ostream &rawOut = this->logger;

    rawOut << Fold("Random and Maximal PACKing PACKage - computational package dedicated to simulate various packing "
                   "models using Monte Carlo method.").width(80) << std::endl;
    rawOut << std::endl;
    rawOut << "Usage: " << this->cmd << " [mode] (mode dependent parameters). " << std::endl;
    rawOut << std::endl;
    rawOut << "Available modes:" << std::endl;
    rawOut << "casino" << std::endl;
    rawOut << Fold("Monte Carlo sampling for both hard and soft potentials.").width(80).margin(4) << std::endl;
    rawOut << "preview" << std::endl;
    rawOut << Fold("Based on the input file generate initial configuration and store it in a given format.")
              .width(80).margin(4) << std::endl;
    rawOut << "shape-preview" << std::endl;
    rawOut << Fold("Provides information and preview for a given shape.").width(80).margin(4) << std::endl;
    rawOut << "trajectory" << std::endl;
    rawOut << Fold("Replays recorded simulation trajectory and performs some operations on it.")
              .width(80).margin(4) << std::endl;
    rawOut << "-v, --version, version" << std::endl;
    rawOut << Fold("Shows current version.").width(80).margin(4) << std::endl;
    rawOut << "-h, --help, help" << std::endl;
    rawOut << Fold("Shows this help.").width(80).margin(4) << std::endl;
    rawOut << std::endl;
    rawOut << "Type " + this->cmd + " [mode] --help to get help on the specific mode." << std::endl;

    return EXIT_SUCCESS;
}
