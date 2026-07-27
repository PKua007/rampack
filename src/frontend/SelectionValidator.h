//
// Created by Codex on 18/07/2026.
//

#ifndef RAMPACK_SELECTIONVALIDATOR_H
#define RAMPACK_SELECTIONVALIDATOR_H

#include <cstddef>
#include <string>

#include "core/ParticleSelectable.h"


class Logger;

class SelectionValidator {
public:
    static void validate(const ParticleSelectable &selectable, const std::string &objectType,
                         const std::string &objectName, std::size_t numParticles, Logger &logger);
};


#endif //RAMPACK_SELECTIONVALIDATOR_H
