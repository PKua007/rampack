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
#include "core/move_samplers/ReflectionSampler.h"
#include "core/geometry/FlipAxis.h"
#include "ParticleSelectionMatcher.h"

using namespace pyon::matcher;


namespace {
    MatcherDataclass create_rototranslation();
    MatcherDataclass create_translation();
    MatcherDataclass create_rotation();
    MatcherDataclass create_axial_rotation();
    MatcherDataclass create_flip();
    MatcherDataclass create_reflection();


    constexpr double AXIS_NORM_EPSILON = 1e-10;
    const auto hasNonZeroNorm = [](const ArrayData &arrayData) {
        return arrayData.asVector<3>().norm2() > AXIS_NORM_EPSILON*AXIS_NORM_EPSILON;
    };
    constexpr auto nonZeroNormDescription = "non-zero norm";

    const auto vectorLabAxisMatcher = MatcherArray(MatcherFloat{}, 3)
        .filter(hasNonZeroNorm)
        .describe(nonZeroNormDescription)
        .mapTo([](const ArrayData &arrayData) -> Vector<3> {
            return arrayData.asVector<3>();
        });
    const auto xyzLabAxisMatcher = MatcherString{}
        .anyOf({"x", "y", "z"})
        .mapTo([](const std::string &axis) -> Vector<3> {
            if (axis == "x")        return {1, 0, 0};
            else if (axis == "y")   return {0, 1, 0};
            else if (axis == "z")   return {0, 0, 1};
            else                    AssertThrow(axis);
        });
    const auto dataclassLabAxisMatcher = MatcherDataclass("lab_coord")
        .arguments({{"axis", vectorLabAxisMatcher | xyzLabAxisMatcher}})
        .mapTo([](const DataclassData &labAxis) -> Vector<3> {
            return labAxis["axis"].as<Vector<3>>();
        });
    const auto labAxisMatcher = vectorLabAxisMatcher | xyzLabAxisMatcher | dataclassLabAxisMatcher;

    const auto vectorShapeAxisMatcher = MatcherArray(MatcherFloat{}, 3)
        .filter(hasNonZeroNorm)
        .describe(nonZeroNormDescription)
        .mapTo([](const ArrayData &arrayData) -> GeneralShapeAxis {
            return GeneralShapeAxis(arrayData.asVector<3>());
        });
    const auto xyzShapeAxisMatcher = MatcherString{}
        .anyOf({"x", "y", "z"})
        .mapTo([](const std::string &axis) -> GeneralShapeAxis {
            if (axis == "x")        return GeneralShapeAxis(Vector<3>{1, 0, 0});
            else if (axis == "y")   return GeneralShapeAxis(Vector<3>{0, 1, 0});
            else if (axis == "z")   return GeneralShapeAxis(Vector<3>{0, 0, 1});
            else                    AssertThrow(axis);
        });
    const auto namedShapeAxisMatcher = MatcherString{}
        .anyOf({"primary", "secondary", "auxiliary"})
        .mapTo([](const std::string &axis) -> GeneralShapeAxis {
            if (axis == "primary")          return ShapeGeometry::Axis::PRIMARY;
            else if (axis == "secondary")   return ShapeGeometry::Axis::SECONDARY;
            else if (axis == "auxiliary")   return ShapeGeometry::Axis::AUXILIARY;
            else                            AssertThrow(axis);
        });
    const auto dataclassShapeAxisMatcher = MatcherDataclass("shape_coord")
        .arguments({{"axis", vectorShapeAxisMatcher | xyzShapeAxisMatcher | namedShapeAxisMatcher}})
        .mapTo([](const DataclassData &shapeAxis) -> GeneralShapeAxis {
            return shapeAxis["axis"].as<GeneralShapeAxis>();
        });
    const auto generalShapeAxisMatcher = namedShapeAxisMatcher | dataclassShapeAxisMatcher;

    [[maybe_unused]] const auto flipSymmetryAxisMatcher = MatcherString{}
        .anyOf({"primary", "secondary", "auxiliary", "orthogonal_to_primary"})
        .mapTo([](const std::string &axis) -> FlipAxis {
            if (axis == "primary")                      return ShapeGeometry::Axis::PRIMARY;
            else if (axis == "secondary")               return ShapeGeometry::Axis::SECONDARY;
            else if (axis == "auxiliary")               return ShapeGeometry::Axis::AUXILIARY;
            else if (axis == "orthogonal_to_primary")   return FlipAxis::orthogonalToPrimaryTag;
            else                                        AssertThrow(axis);
        });

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
                        {"max_trans_step", maxTransStep, "None"},
                        {"whitelist_shapes", ParticleSelectionMatcher::particleMaskMatcher, "None"},
                        {"blacklist_shapes", ParticleSelectionMatcher::particleMaskMatcher, "None"}})
            .filter([](const DataclassData &rototranslation) {
                auto transStep = rototranslation["trans_step"].as<double>();
                auto maxTransStep = rototranslation["max_trans_step"].as<double>();
                if (maxTransStep == 0)
                    return true;
                return transStep <= maxTransStep;
            })
            .describe("if max_trans_step is specified, it has to be >= trans_step")
            .filter(ParticleSelectionMatcher::hasAtMostOneParticleMask)
            .describe(ParticleSelectionMatcher::mutualExclusionDescription)
            .mapTo([](const DataclassData &rototranslation) -> std::shared_ptr<MoveSampler> {
                auto transStep = rototranslation["trans_step"].as<double>();
                auto rotStep = rototranslation["rot_step"].as<std::optional<double>>();
                auto maxTransStep = rototranslation["max_trans_step"].as<double>();
                auto sampler = std::make_shared<RototranslationSampler>(transStep, rotStep, maxTransStep);
                return ParticleSelectionMatcher::apply(std::move(sampler), rototranslation);
            });
    }

    MatcherDataclass create_translation() {
        auto maxTransStepFloat = MatcherFloat{}.positive();
        auto maxTransStepNone = MatcherNone{}.mapTo([]() { return 0.; });
        auto maxTransStep = maxTransStepFloat | maxTransStepNone;

        return MatcherDataclass("translation")
            .arguments({{"step", MatcherFloat{}.positive()},
                        {"max_step", maxTransStep, "None"},
                        {"whitelist_shapes", ParticleSelectionMatcher::particleMaskMatcher, "None"},
                        {"blacklist_shapes", ParticleSelectionMatcher::particleMaskMatcher, "None"}})
            .filter([](const DataclassData &rototranslation) {
                auto transStep = rototranslation["step"].as<double>();
                auto maxTransStep = rototranslation["max_step"].as<double>();
                if (maxTransStep == 0)
                    return true;
                return transStep <= maxTransStep;
            })
            .describe("if max_trans_step is specified, it has to be >= trans_step")
            .filter(ParticleSelectionMatcher::hasAtMostOneParticleMask)
            .describe(ParticleSelectionMatcher::mutualExclusionDescription)
            .mapTo([](const DataclassData &translation) -> std::shared_ptr<MoveSampler> {
                auto transStep = translation["step"].as<double>();
                auto maxTransStep = translation["max_step"].as<double>();
                auto sampler = std::make_shared<TranslationSampler>(transStep, maxTransStep);
                return ParticleSelectionMatcher::apply(std::move(sampler), translation);
            });
    }

    MatcherDataclass create_rotation() {
        return MatcherDataclass("rotation")
            .arguments({{"step", MatcherFloat{}.positive()},
                        {"whitelist_shapes", ParticleSelectionMatcher::particleMaskMatcher, "None"},
                        {"blacklist_shapes", ParticleSelectionMatcher::particleMaskMatcher, "None"}})
            .filter(ParticleSelectionMatcher::hasAtMostOneParticleMask)
            .describe(ParticleSelectionMatcher::mutualExclusionDescription)
            .mapTo([](const DataclassData &rotation) -> std::shared_ptr<MoveSampler> {
                auto step = rotation["step"].as<double>();
                auto sampler = std::make_shared<RotationSampler>(step);
                return ParticleSelectionMatcher::apply(std::move(sampler), rotation);
            });
    }

    MatcherDataclass create_axial_rotation() {
        return MatcherDataclass("axial_rotation")
            .arguments({{"step", MatcherFloat{}.positive()},
                        {"axis", labAxisMatcher | generalShapeAxisMatcher},
                        {"whitelist_shapes", ParticleSelectionMatcher::particleMaskMatcher, "None"},
                        {"blacklist_shapes", ParticleSelectionMatcher::particleMaskMatcher, "None"}})
            .filter(ParticleSelectionMatcher::hasAtMostOneParticleMask)
            .describe(ParticleSelectionMatcher::mutualExclusionDescription)
            .mapTo([](const DataclassData &rotationAroundAxis) -> std::shared_ptr<MoveSampler> {
                auto step = rotationAroundAxis["step"].as<double>();
                const auto &axis = rotationAroundAxis["axis"];
                std::shared_ptr<MoveSampler> sampler;
                if (axis.is<Vector<3>>())
                    sampler = std::make_shared<AxialRotationSampler>(step, axis.as<Vector<3>>());
                else if (axis.is<GeneralShapeAxis>())
                    sampler = std::make_shared<AxialRotationSampler>(step, axis.as<GeneralShapeAxis>());
                else
                    AssertThrow("axis should be Vector<3> or GeneralShapeAxis");
                return ParticleSelectionMatcher::apply(std::move(sampler), rotationAroundAxis);
            });
    }

    MatcherDataclass create_flip() {
        return MatcherDataclass("flip")
            .arguments({{"every", MatcherInt{}.positive().mapTo<std::size_t>(), "10"},
                        {"whitelist_shapes", ParticleSelectionMatcher::particleMaskMatcher, "None"},
                        {"blacklist_shapes", ParticleSelectionMatcher::particleMaskMatcher, "None"}})
            .filter(ParticleSelectionMatcher::hasAtMostOneParticleMask)
            .describe(ParticleSelectionMatcher::mutualExclusionDescription)
            .mapTo([](const DataclassData &flip) -> std::shared_ptr<MoveSampler> {
                auto every = flip["every"].as<std::size_t>();
                auto sampler = std::make_shared<FlipSampler>(every);
                return ParticleSelectionMatcher::apply(std::move(sampler), flip);
            });
    }

    MatcherDataclass create_reflection() {
        return MatcherDataclass("reflection")
            .arguments({{"reflection_axis", labAxisMatcher | generalShapeAxisMatcher},
                {"shape_symmetry_axis", generalShapeAxisMatcher},
                {"every", MatcherInt{}.positive().mapTo<std::size_t>(), "10"},
                {"whitelist_shapes", ParticleSelectionMatcher::particleMaskMatcher, "None"},
                {"blacklist_shapes", ParticleSelectionMatcher::particleMaskMatcher, "None"}})
            .filter(ParticleSelectionMatcher::hasAtMostOneParticleMask)
            .describe(ParticleSelectionMatcher::mutualExclusionDescription)
            .mapTo([](const DataclassData &reflection) -> std::shared_ptr<MoveSampler> {
                const auto &reflectionAxis = reflection["reflection_axis"];
                const auto reflectionSymmetryAxis = reflection["shape_symmetry_axis"].as<GeneralShapeAxis>();
                const auto every = reflection["every"].as<std::size_t>();
                std::shared_ptr<MoveSampler> sampler;
                if (reflectionAxis.is<Vector<3>>()) {
                    sampler = std::make_shared<ReflectionSampler>(reflectionAxis.as<Vector<3>>(),
                                                                  reflectionSymmetryAxis, every);
                } else if (reflectionAxis.is<GeneralShapeAxis>()) {
                    sampler = std::make_shared<ReflectionSampler>(reflectionAxis.as<GeneralShapeAxis>(),
                                                                  reflectionSymmetryAxis, every);
                } else {
                    AssertThrow("reflection_axis should be Vector<3> or GeneralShapeAxis");
                }
                return ParticleSelectionMatcher::apply(std::move(sampler), reflection);
            });
    }
}

MatcherAlternative MoveSamplerMatcher::create() {
    return create_rototranslation()
        | create_translation()
        | create_rotation()
        | create_axial_rotation()
        | create_flip()
        | create_reflection();
}
