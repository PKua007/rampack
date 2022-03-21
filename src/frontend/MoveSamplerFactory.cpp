//
// Created by pkua on 21.03.2022.
//

#include <sstream>

#include "MoveSamplerFactory.h"
#include "utils/Assertions.h"
#include "core/move_samplers/RototranslationSampler.h"


std::unique_ptr<MoveSampler> MoveSamplerFactory::create(const std::string &moveSamplerString) {
    std::istringstream moveSamplerStream(moveSamplerString);
    std::string moveName;
    moveSamplerStream >> moveName;
    ValidateMsg(moveSamplerStream, "Malformed move sampler");

    if (moveName == "rototranslation") {
        double translationStep{};
        double rotationStep{};
        moveSamplerStream >> translationStep >> rotationStep;
        ValidateMsg(moveSamplerStream, "Malformed move sampler");
        Validate(translationStep > 0);
        Validate(rotationStep > 0);

        moveSamplerStream >> std::ws;
        if (moveSamplerStream.eof())
            return std::make_unique<RototranslationSampler>(translationStep, rotationStep);

        double maxTranslationStep{};
        moveSamplerStream >> maxTranslationStep;
        ValidateMsg(moveSamplerStream, "Malformed move sampler");
        Validate(maxTranslationStep > 0);
        Validate(maxTranslationStep >= translationStep);
        return std::make_unique<RototranslationSampler>(translationStep, rotationStep, maxTranslationStep);
    } else {
        throw ValidationException("Unknown move sampler: " + moveName);
    }
}
