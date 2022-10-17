//
// Created by pkua on 21.03.2022.
//

#include <sstream>

#include "MoveSamplerFactory.h"
#include "utils/Assertions.h"
#include "core/move_samplers/RototranslationSampler.h"
#include "core/move_samplers/TranslationSampler.h"
#include "core/move_samplers/RotationSampler.h"
#include "core/move_samplers/FlipSampler.h"
#include "ParseUtils.h"


std::unique_ptr<MoveSampler> MoveSamplerFactory::create(const std::string &moveSamplerString,
                                                        const ShapeTraits &traits)
{
    std::istringstream moveSamplerStream(moveSamplerString);
    std::string moveName;
    moveSamplerStream >> moveName;
    ValidateMsg(moveSamplerStream, "Malformed move sampler");

    if (moveName == "rototranslation") {
        double translationStep{};
        moveSamplerStream >> translationStep;
        ValidateMsg(moveSamplerStream, "Malformed move sampler");
        Validate(translationStep > 0);
        if (!ParseUtils::isAnythingLeft(moveSamplerStream))
            return std::make_unique<RototranslationSampler>(traits.getInteraction(), translationStep);

        double rotationStep{};
        moveSamplerStream >> translationStep;
        Validate(rotationStep > 0);
        if (!ParseUtils::isAnythingLeft(moveSamplerStream))
            return std::make_unique<RototranslationSampler>(translationStep, rotationStep);

        double maxTranslationStep{};
        moveSamplerStream >> maxTranslationStep;
        ValidateMsg(moveSamplerStream, "Malformed move sampler");
        Validate(maxTranslationStep > 0);
        Validate(maxTranslationStep >= translationStep);
        return std::make_unique<RototranslationSampler>(translationStep, rotationStep, maxTranslationStep);
    } else if (moveName == "translation") {
        double translationStep{};
        moveSamplerStream >> translationStep;
        ValidateMsg(moveSamplerStream, "Malformed move sampler");
        Validate(translationStep > 0);

        moveSamplerStream >> std::ws;
        if (moveSamplerStream.eof())
            return std::make_unique<TranslationSampler>(translationStep);

        double maxTranslationStep{};
        moveSamplerStream >> maxTranslationStep;
        ValidateMsg(moveSamplerStream, "Malformed move sampler");
        Validate(maxTranslationStep > 0);
        Validate(maxTranslationStep >= translationStep);
        return std::make_unique<TranslationSampler>(translationStep, maxTranslationStep);
    } else if (moveName == "rotation") {
        double rotationStep{};
        moveSamplerStream >> rotationStep;
        ValidateMsg(moveSamplerStream, "Malformed move sampler");
        Validate(rotationStep > 0);
        return std::make_unique<RotationSampler>(rotationStep);
    } else if (moveName == "flip") {
        std::size_t flipEvery{};

        moveSamplerStream >> std::ws;
        if (moveSamplerStream.eof()) {
            flipEvery = 10;
        } else {
            moveSamplerStream >> flipEvery;
            ValidateMsg(moveSamplerStream, "Malformed move sampler");
            Validate(flipEvery > 0);
        }

        return std::make_unique<FlipSampler>(flipEvery);
    } else {
        throw ValidationException("Unknown move sampler: " + moveName);
    }
}
