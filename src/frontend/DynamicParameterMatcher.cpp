//
// Created by Piotr Kubala on 07/01/2023.
//

#include "DynamicParameterMatcher.h"
#include "core/dynamic_parameters/ConstantDynamicParameter.h"
#include "core/dynamic_parameters/LinearDynamicParameter.h"
#include "core/dynamic_parameters/ExponentialDynamicParameter.h"
#include "core/dynamic_parameters/PiecewiseDynamicParameter.h"


using namespace pyon::matcher;


namespace {
    MatcherAlternative create_specific();
    MatcherAlternative create_const();
    MatcherDataclass create_linear();
    MatcherDataclass create_exp();
    MatcherDataclass create_piecewise();


    MatcherAlternative create_specific() {
        return create_const() | create_linear() | create_exp();
    }

    MatcherAlternative create_const() {
        auto constFloat = MatcherFloat{}.mapTo([](double d) -> std::shared_ptr<DynamicParameter> {
            return std::make_shared<ConstantDynamicParameter>(d);
        });

        auto constDataclass = MatcherDataclass("const")
            .arguments({{"value", MatcherFloat{}}})
            .mapTo([](const DataclassData &const_) -> std::shared_ptr<DynamicParameter> {
                auto value = const_["value"].as<double>();
                return std::make_shared<ConstantDynamicParameter>(value);
            });

        return constFloat | constDataclass;
    }

    MatcherDataclass create_linear() {
        return MatcherDataclass("linear")
            .arguments({{"slope", MatcherFloat{}},
                        {"intercept", MatcherFloat{}, "0"}})
            .mapTo([](const DataclassData &linear) -> std::shared_ptr<DynamicParameter> {
                auto slope = linear["slope"].as<double>();
                auto intercept = linear["intercept"].as<double>();
                return std::make_shared<LinearDynamicParameter>(slope, intercept);
            });
    }

    MatcherDataclass create_exp() {
        return MatcherDataclass("exp")
            .arguments({{"a0", MatcherFloat{}},
                        {"rate", MatcherFloat{}}})
            .mapTo([](const DataclassData &exp) -> std::shared_ptr<DynamicParameter> {
                auto a0 = exp["a0"].as<double>();
                auto rate = exp["rate"].as<double>();
                return std::make_shared<ExponentialDynamicParameter>(a0, rate);
            });
    }

    MatcherDataclass create_piecewise() {
        using CycleParam = std::pair<std::size_t, std::shared_ptr<DynamicParameter>>;

        auto piece = MatcherDataclass("piece")
            .arguments({{"start", MatcherInt{}.nonNegative().mapTo<std::size_t>()},
                        {"param", create_specific()}})
            .mapTo([](const DataclassData &piece) -> CycleParam {
                auto start = piece["start"].as<std::size_t>();
                auto param = piece["param"].as<std::shared_ptr<DynamicParameter>>();
                return {start, param};
            });

        auto pieceArray = MatcherArray{}
            .elementsMatch(piece)
            .sizeAtLeast(2)
            .mapToStdVector<CycleParam>();

        return MatcherDataclass("piecewise")
            .variadicArguments(pieceArray)
            .filter([](const DataclassData &piecewise) {
                auto pieces = piecewise.getVariadicArguments().asStdVector<CycleParam>();
                if (pieces.front().first != 0)
                    return false;

                for (std::size_t i{}; i < pieces.size() - 1; i++)
                    if (pieces[i].first >= pieces[i + 1].first)
                        return false;

                return true;
            })
            .describe("with cycle numbers starting from 0 in an ascending order")
            .mapTo([](const DataclassData &piecewise) -> std::shared_ptr<DynamicParameter> {
                auto pieces = piecewise.getVariadicArguments().asStdVector<CycleParam>();
                return std::make_shared<PiecewiseDynamicParameter>(std::move(pieces));
            });
    }
}


pyon::matcher::MatcherAlternative DynamicParameterMatcher::create() {
    return create_specific() | create_piecewise();
}
