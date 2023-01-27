//
// Created by Piotr Kubala on 06/01/2023.
//

#include <memory>

#include "BoxScalerMatcher.h"

#include "core/TriclinicBoxScaler.h"

#include "core/volume_scalers/TriclinicAdapter.h"
#include "core/volume_scalers/DeltaVolumeScaler.h"
#include "core/volume_scalers/AnisotropicVolumeScaler.h"
#include "core/volume_scalers/LinearScalingFactorSampler.h"
#include "core/volume_scalers/LogScalingFactorSampler.h"
#include "core/volume_scalers/TriclinicDeltaScaler.h"

#include "frontend/ScalingDirectionParser.h"


using namespace pyon::matcher;


namespace {
    using FactorSamplerFactory = std::function<std::unique_ptr<ScalingFactorSampler>()>;


    MatcherDataclass create_delta_v();
    MatcherDataclass create_anisotropic(const std::string &className, const FactorSamplerFactory &factorSamplerFactory);
    MatcherDataclass create_linear();
    MatcherDataclass create_log();
    MatcherDataclass create_delta_triclinic();


    const auto &X = AnisotropicVolumeScaler::X;
    const auto &Y = AnisotropicVolumeScaler::Y;
    const auto &Z = AnisotropicVolumeScaler::Z;

    auto isotropic = MatcherString("isotropic").mapTo([](const std::string&) { return X & Y & Z; });
    auto anisotropicX = MatcherString("anisotropic x").mapTo([](const std::string&) { return X | (Y & Z); });
    auto anisotropicY = MatcherString("anisotropic y").mapTo([](const std::string&) { return Y | (Z & X); });
    auto anisotropicZ = MatcherString("anisotropic z").mapTo([](const std::string&) { return Z | (X & Y); });
    auto anisotropicXYZ = MatcherString("anisotropic xyz").mapTo([](const std::string&) { return X | Y | Z; });
    auto predefinedDirection = isotropic | anisotropicX | anisotropicY | anisotropicZ | anisotropicXYZ;

    // TODO: MatchException
    auto manualDirection = MatcherString{}
        .containsOnlyCharacters("xyz()[]")
        .filter([](const std::string &directions) {
            auto count = [&directions](char x) { return std::count(directions.begin(), directions.end(), x); };
            return count('x') == 1 && count('y') == 1 && count('z') == 1;
        })
        .describe("with each direction specified exactly once")
        .mapTo([](const std::string &directions) {
            return ScalingDirectionParser::parse(directions);
        });

    auto scalingDirection = predefinedDirection | manualDirection;


    MatcherDataclass create_delta_v() {
        return MatcherDataclass("delta_v")
            .arguments({{"step", MatcherFloat{}.positive()}})
            .mapTo([](const DataclassData &deltaV) -> std::shared_ptr<TriclinicBoxScaler> {
                auto step = deltaV["step"].as<double>();
                auto scaler = std::make_unique<DeltaVolumeScaler>();
                return std::make_shared<TriclinicAdapter>(std::move(scaler), step);
            });
    }

    MatcherDataclass create_anisotropic(const std::string &className,
                                        const FactorSamplerFactory &factorSamplerFactory)
    {
        return MatcherDataclass(className)
            .arguments({{"spec", scalingDirection},
                        {"step", MatcherFloat{}.positive()},
                        {"independent", MatcherBoolean{}, "False"}})
            .mapTo([className, factorSamplerFactory](const DataclassData &linear) -> std::shared_ptr<TriclinicBoxScaler> {
                auto scalingDirection = linear["spec"].as<AnisotropicVolumeScaler::ScalingDirection>();
                auto step = linear["step"].as<double>();
                auto independent = linear["independent"].as<bool>();
                auto factorSampler = factorSamplerFactory();
                auto scaler = std::make_unique<AnisotropicVolumeScaler>(
                    std::move(factorSampler), scalingDirection, independent
                );
                return std::make_shared<TriclinicAdapter>(std::move(scaler), step);
            });
    }

    MatcherDataclass create_linear() {
        return create_anisotropic("linear", []() { return std::make_unique<LinearScalingFactorSampler>(); });
    }

    MatcherDataclass create_log() {
        return create_anisotropic("log", []() { return std::make_unique<LogScalingFactorSampler>(); });
    }

    MatcherDataclass create_delta_triclinic() {
        return MatcherDataclass("delta_triclinic")
            .arguments({{"step", MatcherFloat{}.positive()},
                        {"independent", MatcherBoolean{}, "False"}})
            .mapTo([](const DataclassData &deltaTriclinic) -> std::shared_ptr<TriclinicBoxScaler> {
                auto step = deltaTriclinic["step"].as<double>();
                auto independent = deltaTriclinic["independent"].as<bool>();
                return std::make_shared<TriclinicDeltaScaler>(step, !independent);
            });
    }
}


MatcherAlternative BoxScalerMatcher::create() {
    return create_delta_v() | create_linear() | create_log() | create_delta_triclinic();
}
