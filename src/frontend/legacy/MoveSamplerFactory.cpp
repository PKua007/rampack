//
// Created by pkua on 21.03.2022.
//

#include <sstream>

#include "MoveSamplerFactory.h"
#include "utils/Exceptions.h"
#include "core/move_samplers/RototranslationSampler.h"
#include "core/move_samplers/TranslationSampler.h"
#include "core/move_samplers/RotationSampler.h"
#include "core/move_samplers/FlipSampler.h"
#include "utils/ParseUtils.h"


namespace legacy {
    std::unique_ptr<MoveSampler> MoveSamplerFactory::create(const std::string &moveSamplerString) {
        std::istringstream moveSamplerStream(moveSamplerString);
        std::string moveName;
        moveSamplerStream >> moveName;
        ValidateMsg(moveSamplerStream, "Malformed move sampler");

        if (moveName == "rototranslation") {
            double translationStep{};
            moveSamplerStream >> translationStep;
            ValidateMsg(moveSamplerStream, "Malformed move sampler");
            ValidateMsg(translationStep > 0, "Translation step shoul be positive");
            if (!ParseUtils::isAnythingLeft(moveSamplerStream))
                return std::make_unique<RototranslationSampler>(translationStep, std::nullopt);

            double rotationStep{};
            moveSamplerStream >> rotationStep;
            ValidateMsg(rotationStep > 0, "Rotation step should be positive");
            if (!ParseUtils::isAnythingLeft(moveSamplerStream))
                return std::make_unique<RototranslationSampler>(translationStep, rotationStep);

            double maxTranslationStep{};
            moveSamplerStream >> maxTranslationStep;
            ValidateMsg(moveSamplerStream, "Malformed move sampler");
            ValidateMsg(maxTranslationStep > 0, "Max translation step should be positive");
            ValidateMsg(maxTranslationStep >= translationStep,
                        "Max translation step should be not smaller than translation step");
            return std::make_unique<RototranslationSampler>(translationStep, rotationStep, maxTranslationStep);
        } else if (moveName == "translation") {
            double translationStep{};
            moveSamplerStream >> translationStep;
            ValidateMsg(moveSamplerStream, "Malformed move sampler");
            ValidateMsg(translationStep > 0, "Translation step should be positive");

            moveSamplerStream >> std::ws;
            if (moveSamplerStream.eof())
                return std::make_unique<TranslationSampler>(translationStep);

            double maxTranslationStep{};
            moveSamplerStream >> maxTranslationStep;
            ValidateMsg(moveSamplerStream, "Malformed move sampler");
            ValidateMsg(maxTranslationStep > 0, "Max translation step should be positive");
            ValidateMsg(maxTranslationStep >= translationStep,
                        "Max translation step should be not smaller than translation step");
            return std::make_unique<TranslationSampler>(translationStep, maxTranslationStep);
        } else if (moveName == "rotation") {
            double rotationStep{};
            moveSamplerStream >> rotationStep;
            ValidateMsg(moveSamplerStream, "Malformed move sampler");
            ValidateMsg(rotationStep > 0, "Rotation step should be positive");
            return std::make_unique<RotationSampler>(rotationStep);
        } else if (moveName == "flip") {
            std::size_t flipEvery{};

            moveSamplerStream >> std::ws;
            if (moveSamplerStream.eof()) {
                flipEvery = 10;
            } else {
                moveSamplerStream >> flipEvery;
                ValidateMsg(moveSamplerStream, "Malformed move sampler");
                ValidateMsg(flipEvery > 0, "Flip every parameter should be positive");
            }

            return std::make_unique<FlipSampler>(flipEvery);
        } else {
            throw InputError("Unknown move sampler: " + moveName);
        }
    }
}
