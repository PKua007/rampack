//
// Created by Piotr Kubala on 15/02/2023.
//

#include "VersionMode.h"
#include "utils/Version.h"


int VersionMode::main([[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
    std::ostream &rawOut = this->logger;

    rawOut << "RAMPACK v" << CURRENT_VERSION << std::endl;

    return 0;
}
