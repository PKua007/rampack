//
// Created by Piotr Kubala on 05/01/2023.
//

#include "MoveSamplerMatcher.h"
#include "core/MoveSampler.h"
#include "core/move_samplers/RototranslationSampler.h"
#include "core/move_samplers/TranslationSampler.h"
#include "core/move_samplers/RotationSampler.h"
#include "core/move_samplers/FlipSampler.h"

using namespace pyon::matcher;


namespace {
    MatcherDataclass create_rototranslation();
    MatcherDataclass create_translation();
    MatcherDataclass create_rotation();
    MatcherDataclass create_flip();


    MatcherDataclass create_rototranslation() {
        auto rotStepFloat = MatcherFloat{}.positive().mapTo([](double step) -> std::optional<double> { return step; });
        auto rotStepAuto = MatcherString("auto")
            .mapTo([](const std::string&) -> std::optional<double> { return std::nullopt; });
        auto rotStep = rotStepFloat | rotStepAuto;

        auto maxTransStepFloat = MatcherFloat{}.positive();
        auto maxTransStepNone = MatcherNone{}.mapTo([]() { return 0.; });
        auto maxTransStep = maxTransStepFloat | maxTransStepNone;

        return MatcherDataclass("rototranslation")
            .arguments({{"trans_step", MatcherFloat{}.positive()},
                        {"rot_step", rotStep, R"("auto")"},
                        {"max_trans_step", maxTransStep, "None"}})
            .filter([](const DataclassData &rototranslation) {
                auto transStep = rototranslation["trans_step"].as<double>();
                auto maxTransStep = rototranslation["max_trans_step"].as<double>();
                if (maxTransStep == 0)
                    return true;
                return transStep <= maxTransStep;
            })
            .describe("if max_trans_step is specified, it has to be >= trans_step")
            .mapTo([](const DataclassData &rototranslation) -> std::shared_ptr<MoveSampler> {
                auto transStep = rototranslation["trans_step"].as<double>();
                auto rotStep = rototranslation["rot_step"].as<std::optional<double>>();
                auto maxTransStep = rototranslation["max_trans_step"].as<double>();
                return std::make_shared<RototranslationSampler>(transStep, rotStep, maxTransStep);
            });
    }

    MatcherDataclass create_translation() {
        auto maxTransStepFloat = MatcherFloat{}.positive();
        auto maxTransStepNone = MatcherNone{}.mapTo([]() { return 0.; });
        auto maxTransStep = maxTransStepFloat | maxTransStepNone;

        return MatcherDataclass("translation")
            .arguments({{"step", MatcherFloat{}.positive()},
                        {"max_step", maxTransStep, "None"}})
            .filter([](const DataclassData &rototranslation) {
                auto transStep = rototranslation["step"].as<double>();
                auto maxTransStep = rototranslation["max_step"].as<double>();
                if (maxTransStep == 0)
                    return true;
                return transStep <= maxTransStep;
            })
            .describe("if max_trans_step is specified, it has to be >= trans_step")
            .mapTo([](const DataclassData &translation) -> std::shared_ptr<MoveSampler> {
                auto transStep = translation["step"].as<double>();
                auto maxTransStep = translation["max_step"].as<double>();
                return std::make_shared<TranslationSampler>(transStep, maxTransStep);
            });
    }

    MatcherDataclass create_rotation() {
        return MatcherDataclass("rotation")
            .arguments({{"step", MatcherFloat{}.positive()}})
            .mapTo([](const DataclassData &rotation) -> std::shared_ptr<MoveSampler> {
                auto step = rotation["step"].as<double>();
                return std::make_shared<RotationSampler>(step);
            });
    }

    MatcherDataclass create_flip() {
        return MatcherDataclass("flip")
            .arguments({{"every", MatcherInt{}.positive().mapTo<std::size_t>(), "10"}})
            .mapTo([](const DataclassData &flip) -> std::shared_ptr<MoveSampler> {
                auto every = flip["every"].as<std::size_t>();
                return std::make_shared<FlipSampler>(every);
            });
    }
}


MatcherAlternative MoveSamplerMatcher::create() {
    return create_rototranslation() | create_translation() | create_rotation() | create_flip();
}
