//
// Created by Codex on 04/07/2026.
//

#ifndef RAMPACK_MOVESELECTIONVALIDATOR_H
#define RAMPACK_MOVESELECTIONVALIDATOR_H

#include <cstddef>

#include "core/Simulation.h"


class Logger;

class MoveSelectionValidator {
public:
    static void validate(const Simulation::Environment &environment, std::size_t numParticles, Logger &logger);
};


#endif //RAMPACK_MOVESELECTIONVALIDATOR_H
