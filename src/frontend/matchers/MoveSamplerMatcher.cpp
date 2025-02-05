//
// Created by Piotr Kubala on 05/01/2023.
//

#include "MoveSamplerMatcher.h"
#include "core/MoveSampler.h"
#include "core/move_samplers/RototranslationSampler.h"
#include "core/move_samplers/TranslationSampler.h"
#include "core/move_samplers/RotationSampler.h"
#include "core/move_samplers/AxialRotationSampler.h"
#include "core/move_samplers/FlipSampler.h"
#include "core/move_samplers//ReflectionSampler.h"

using namespace pyon::matcher;


namespace {
    MatcherDataclass create_rototranslation();
    MatcherDataclass create_translation();
    MatcherDataclass create_rotation();
    MatcherDataclass create_axis_rotation();
    MatcherDataclass create_flip();
    MatcherDataclass create_reflection();


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

    MatcherDataclass create_axis_rotation() {
        using Axis = AxialRotationSampler::Axis;

        auto axisArray = MatcherArray(MatcherFloat{}, 3)
            .filter([](const ArrayData &arrayData) {
                return arrayData.asVector<3>().norm2() > 1e-20;
            })
            .describe("non-zero norm")
            .mapTo([](const ArrayData &arrayData) -> Axis {
                return arrayData.asVector<3>();
            });
        auto axisString = MatcherString{}
            .anyOf({"x", "y", "z", "primary", "secondary", "auxiliary"})
            .mapTo([](const std::string &axis) -> Axis {
                if (axis == "x")                return Vector<3>{1, 0, 0};
                else if (axis == "y")           return Vector<3>{0, 1, 0};
                else if (axis == "z")           return Vector<3>{0, 0, 1};
                else if (axis == "primary")     return ShapeGeometry::Axis::PRIMARY;
                else if (axis == "secondary")   return ShapeGeometry::Axis::SECONDARY;
                else if (axis == "auxiliary")   return ShapeGeometry::Axis::AUXILIARY;
                else                            AssertThrow(axis);
            });
        auto rotAxis = axisArray | axisString;

        return MatcherDataclass("axial_rotation")
            .arguments({{"step", MatcherFloat{}.positive()},
                        {"axis", rotAxis}})
            .mapTo([](const DataclassData &rotationAroundAxis) -> std::shared_ptr<MoveSampler> {
                auto step = rotationAroundAxis["step"].as<double>();
                auto axis = rotationAroundAxis["axis"].as<Axis>();
                return std::make_shared<AxialRotationSampler>(step, axis);
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

    MatcherDataclass create_reflection() {
        return MatcherDataclass("reflection")
                .arguments({{"every", MatcherInt{}.positive().mapTo<std::size_t>(), "10"},
                            {"planeAxis", MatcherArray(MatcherFloat{}.mapTo<double>(),3), "[0,0,1]"}})
                .mapTo([](const DataclassData &reflection) -> std::shared_ptr<MoveSampler> {
                    auto every = reflection["every"].as<std::size_t>();
                    auto planeAxisData = reflection["planeAxis"].as<pyon::matcher::ArrayData>();
                    auto planeAxis = Vector<3, double>({planeAxisData[0].as<double>(), planeAxisData[1].as<double>(), planeAxisData[2].as<double>()});
                    return std::make_shared<ReflectionSampler>(every, planeAxis);
                });
    }
}

MatcherAlternative MoveSamplerMatcher::create() {
    return create_rototranslation() | create_translation() | create_rotation() | create_axis_rotation() | create_flip() | create_reflection();
}
