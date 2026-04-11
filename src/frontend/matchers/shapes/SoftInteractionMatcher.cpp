//
// Created by Codex on 05/04/2026.
//

#include "SoftInteractionMatcher.h"

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "core/interactions/CentralInteraction.h"
#include "core/interactions/LennardJonesInteraction.h"
#include "core/interactions/RepulsiveLennardJonesInteraction.h"
#include "core/interactions/SquareInverseCoreInteraction.h"
#include "utils/Utils.h"


using namespace pyon::matcher;

// TYPES
namespace {
    using ParameterValues = std::map<std::string, Any>;
    template <typename PairData>
    using PairDataFactory = std::function<PairData(const ParameterValues &)>;
    using SupportsTypesFunctor = std::function<bool(const std::vector<std::string> &)>;
    using CreateForTypesFunctor
        = std::function<std::shared_ptr<CentralInteractionBase>(const std::vector<std::string> &)>;

    struct TypeNamePair {
        std::string type1;
        std::string type2;

        TypeNamePair(std::string type1, std::string type2) : type1{std::move(type1)}, type2{std::move(type2)} {
            if (this->type2 < this->type1)
                std::swap(this->type1, this->type2);
        }

        friend bool operator<(const TypeNamePair &lhs, const TypeNamePair &rhs) {
            return std::tie(lhs.type1, lhs.type2) < std::tie(rhs.type1, rhs.type2);
        }

        friend bool operator==(const TypeNamePair &lhs, const TypeNamePair &rhs) {
            return std::tie(lhs.type1, lhs.type2) == std::tie(rhs.type1, rhs.type2);
        }
    };

    using PairParamMap = std::map<TypeNamePair, ParameterValues>;

    class BuiltinSoftInteractionFactory : public SoftInteractionFactory {
    private:
        SupportsTypesFunctor supportsTypes_;
        CreateForTypesFunctor createForTypes_;

    public:
        BuiltinSoftInteractionFactory(SupportsTypesFunctor supportsTypes, CreateForTypesFunctor createForTypes)
            : supportsTypes_{std::move(supportsTypes)}, createForTypes_{std::move(createForTypes)}
        { }

        [[nodiscard]] bool supportsTypes(const std::vector<std::string> &typeLabels) const override {
            return this->supportsTypes_(typeLabels);
        }

        [[nodiscard]] std::shared_ptr<CentralInteractionBase>
        createForTypes(const std::vector<std::string> &typeLabels) const override
        {
            return this->createForTypes_(typeLabels);
        }
    };

    class ErasedMatcher : public MatcherBase {
    private:
        std::shared_ptr<MatcherBase> matcher;

    public:
        explicit ErasedMatcher(std::shared_ptr<MatcherBase> matcher) : matcher{std::move(matcher)}
        { }

        MatchReport match(std::shared_ptr<const pyon::ast::Node> node, Any &result) const override {
            return this->matcher->match(std::move(node), result);
        }

        [[nodiscard]] bool matchNodeType(pyon::ast::Node::Type type) const override {
            return this->matcher->matchNodeType(type);
        }

        [[nodiscard]] std::string outline(std::size_t indent) const override {
            return this->matcher->outline(indent);
        }

        [[nodiscard]] std::string synopsis() const override {
            return this->matcher->synopsis();
        }
    };

    struct InteractionParamSpec {
        std::string name;
        std::shared_ptr<MatcherBase> matcher;
    };
}

// FORWARD DECLARATIONS
namespace {
    template <typename ConcreteMatcher>
    InteractionParamSpec make_parameter_spec(std::string name, ConcreteMatcher matcher);

    template <typename PairData, typename Interaction>
    std::shared_ptr<CentralInteractionBase>
    create_central_interaction(const PairParamMap &pairParameters,
                               const std::optional<ParameterValues> &defaultParameters,
                               const std::vector<std::string> &typeLabels,
                               const PairDataFactory<PairData> &pairDataFactory);

    MatcherDictionary create_param_dictionary_matcher(const std::vector<InteractionParamSpec> &parameterSpecs);
    MatcherDictionary create_type_pair_dictionary_matcher(const std::vector<InteractionParamSpec> &parameterSpecs);
    bool is_valid_type_pair_key(const std::string &key);
    bool has_non_colliding_type_pair_keys(const DictionaryData &dict);
    bool are_type_labels_unique(const std::vector<std::string> &typeLabels);
    bool supports_type_pairs(const PairParamMap &pairParameters,
                             const std::optional<ParameterValues> &defaultParameters,
                             const std::vector<std::string> &typeLabels);
    TypeNamePair parse_type_pair_key(const std::string &key);

    MatcherAlternative create_lj_matcher();
    MatcherAlternative create_wca_matcher();
    MatcherAlternative create_square_inverse_core_matcher();
}

// DEFINITIONS
namespace {
    template <typename ConcreteMatcher>
    InteractionParamSpec make_parameter_spec(std::string name, ConcreteMatcher matcher) {
        static_assert(std::is_base_of_v<MatcherBase, ConcreteMatcher>,
                      "ConcreteMatcher template parameter is not a matcher derived from MatcherBase");
        return {std::move(name), std::make_shared<ConcreteMatcher>(std::move(matcher))};
    }

    template <typename PairData, typename ConcreteInteraction>
    std::shared_ptr<CentralInteractionBase>
    create_central_interaction(const PairParamMap &pairParameters,
                               const std::optional<ParameterValues> &defaultParameters,
                               const std::vector<std::string> &typeLabels,
                               const PairDataFactory<PairData> &pairDataFactory)
    {
        std::map<std::string, std::size_t> typeNameToIdx;
        for (std::size_t i{}; i < typeLabels.size(); i++) {
            [[maybe_unused]] auto [it, inserted] = typeNameToIdx.emplace(typeLabels[i], i);
            Assert(inserted);
        }

        CentrePairDataMap<PairData> pairDataMap(typeLabels.size());
        using CharInsteadOfBoolToAvoidStdVectorBool = char;
        CentrePairDataMap<CharInsteadOfBoolToAvoidStdVectorBool> visitedMap(typeLabels.size());
        for (const auto &[typePair, parameters] : pairParameters) {
            auto type1It = typeNameToIdx.find(typePair.type1);
            auto type2It = typeNameToIdx.find(typePair.type2);
            Assert(type1It != typeNameToIdx.end());
            Assert(type2It != typeNameToIdx.end());

            pairDataMap.setPairData(type1It->second, type2It->second, pairDataFactory(parameters));
            visitedMap.setPairData(type1It->second, type2It->second, 1);
        }

        for (std::size_t i{}; i < typeLabels.size(); i++) {
            for (std::size_t j = i; j < typeLabels.size(); j++) {
                if (visitedMap.getPairData(i, j) == 1)
                    continue;
                if (!defaultParameters)
                    AssertThrow("missing params for: " + typeLabels[i] + " " + typeLabels[j]);

                pairDataMap.setPairData(i, j, pairDataFactory(*defaultParameters));
            }
        }

        return std::make_shared<ConcreteInteraction>(pairDataMap);
    }

    template <typename PairData, typename Interaction>
    MatcherDataclass create_legacy_central_interaction_matcher(
        const std::string &className,
        const std::vector<InteractionParamSpec> &parameterSpecs,
        const PairDataFactory<PairData> &pairDataFactory)
    {
        std::vector<StandardArgumentSpecification> uniformArguments;
        uniformArguments.reserve(parameterSpecs.size());
        for (const auto &parameterSpec : parameterSpecs)
            uniformArguments.emplace_back(parameterSpec.name, ErasedMatcher{parameterSpec.matcher});

        return MatcherDataclass(className)
            .arguments(uniformArguments)
            .mapTo([pairDataFactory](const DataclassData &dataclass) -> std::shared_ptr<SoftInteractionFactory> {
                ParameterValues parameterValues;
                for (const auto &argument : dataclass.getStandardArguments())
                    parameterValues.emplace(argument.name, argument.value);

                SupportsTypesFunctor supportsAllTypes = [](const std::vector<std::string> &) { return true; };
                CreateForTypesFunctor createForTypes =
                    [parameterValues, pairDataFactory](const std::vector<std::string> &) {
                        return std::make_shared<Interaction>(pairDataFactory(parameterValues));
                    };
                return std::make_shared<BuiltinSoftInteractionFactory>(
                    std::move(supportsAllTypes), std::move(createForTypes)
                );
            });
    }

    template <typename PairData, typename Interaction>
    MatcherDataclass
    create_pairwise_central_interaction_matcher(const std::string &className,
                                                const std::vector<InteractionParamSpec> &parameterSpecs,
                                                const PairDataFactory<PairData> &pairDataFactory)
    {
        auto pairDictionaryMatcher = create_type_pair_dictionary_matcher(parameterSpecs);
        return MatcherDataclass(className)
            .arguments({{"params", pairDictionaryMatcher}})
            .mapTo([pairDataFactory](const DataclassData &dataclass) -> std::shared_ptr<SoftInteractionFactory> {
                PairParamMap pairParameters;
                std::optional<ParameterValues> defaultParameters;
                std::tie(pairParameters, defaultParameters)
                    = dataclass["params"].as<std::pair<PairParamMap, std::optional<ParameterValues>>>();

                SupportsTypesFunctor supportsTypePairs =
                    [pairParameters, defaultParameters](const std::vector<std::string> &typeLabels) {
                        return supports_type_pairs(pairParameters, defaultParameters, typeLabels);
                    };
                CreateForTypesFunctor createForTypes =
                    [pairParameters, defaultParameters, pairDataFactory](const std::vector<std::string> &typeLabels) {
                        return create_central_interaction<PairData, Interaction>(
                            pairParameters, defaultParameters, typeLabels, pairDataFactory
                        );
                    };
                return std::make_shared<BuiltinSoftInteractionFactory>(
                    std::move(supportsTypePairs), std::move(createForTypes)
                );
            });
    }

    template <typename PairData, typename Interaction>
    MatcherAlternative create_soft_interaction_matcher(
        const std::string &className,
        const std::vector<InteractionParamSpec> &parameterSpecs,
        const std::function<PairData(const ParameterValues &)> &pairDataFactory)
    {
        auto legacyMatcher = create_legacy_central_interaction_matcher<PairData, Interaction>(
            className, parameterSpecs, pairDataFactory
        );
        auto pairwiseMatcher = create_pairwise_central_interaction_matcher<PairData, Interaction>(
            className, parameterSpecs, pairDataFactory
        );
        return legacyMatcher | pairwiseMatcher;
    }

    MatcherDictionary create_param_dictionary_matcher(const std::vector<InteractionParamSpec> &parameterSpecs) {
        MatcherDictionary matcher;
        std::vector<std::string> parameterNames;
        parameterNames.reserve(parameterSpecs.size());
        for (const auto &parameterSpec : parameterSpecs) {
            matcher.valueAtKeyMatches(parameterSpec.name, ErasedMatcher{parameterSpec.matcher});
            parameterNames.push_back(parameterSpec.name);
        }

        return matcher.hasKeys(parameterNames)
            .hasOnlyKeys(parameterNames)
            .mapTo([](const DictionaryData &dict) -> ParameterValues {
                ParameterValues result;
                for (const auto &[key, value] : dict)
                    result.emplace(key, value);
                return result;
            });
    }

    bool is_valid_type_pair_key(const std::string &key) {
        if (key == "default")
            return true;

        auto typeNames = explode(key, ',');
        if (typeNames.size() != 2)
            return false;

        trim(typeNames[0]);
        trim(typeNames[1]);
        return !typeNames[0].empty() && !typeNames[1].empty();
    }

    TypeNamePair parse_type_pair_key(const std::string &key) {
        auto typeNames = explode(key, ',');
        Expects(typeNames.size() == 2);
        trim(typeNames[0]);
        trim(typeNames[1]);
        Expects(!typeNames[0].empty());
        Expects(!typeNames[1].empty());
        return {std::move(typeNames[0]), std::move(typeNames[1])};
    }

    bool has_non_colliding_type_pair_keys(const DictionaryData &dict) {
        std::set<TypeNamePair> canonicalKeys;
        for ([[maybe_unused]] const auto &[key, value] : dict) {
            if (key == "default")
                continue;

            auto canonicalKey = parse_type_pair_key(key);
            [[maybe_unused]] auto [it, inserted] = canonicalKeys.emplace(canonicalKey);
            if (!inserted)
                return false;
        }

        return true;
    }

    bool are_type_labels_unique(const std::vector<std::string> &typeLabels) {
        std::set<std::string> uniqueLabels(typeLabels.begin(), typeLabels.end());
        return uniqueLabels.size() == typeLabels.size();
    }

    bool supports_type_pairs(const PairParamMap &pairParameters,
                             const std::optional<ParameterValues> &defaultParameters,
                             const std::vector<std::string> &typeLabels)
    {
        Expects(are_type_labels_unique(typeLabels));

        std::set<std::string> knownTypes(typeLabels.begin(), typeLabels.end());
        for ([[maybe_unused]] const auto &[typePair, parameters] : pairParameters)
            if (!knownTypes.count(typePair.type1) || !knownTypes.count(typePair.type2))
                return false;

        if (defaultParameters.has_value())
            return true;

        for (std::size_t i{}; i < typeLabels.size(); i++)
            for (std::size_t j = i; j < typeLabels.size(); j++)
                if (!pairParameters.count(TypeNamePair{typeLabels[i], typeLabels[j]}))
                    return false;

        return true;
    }

    MatcherDictionary create_type_pair_dictionary_matcher(const std::vector<InteractionParamSpec> &parameterSpecs) {
        auto parameterValues = create_param_dictionary_matcher(parameterSpecs);

        return MatcherDictionary{}
            .nonEmpty()
            .keysMatch(is_valid_type_pair_key)
            .describe(R"(keys must be "default" or of the form "type1,type2" with non-empty type labels)")
            .valuesMatch(parameterValues)
            .filter(has_non_colliding_type_pair_keys)
            .describe(R"(type-pair keys must not collide after canonicalization, e.g. "A,B" and "B,A")")
            .mapTo([](const DictionaryData &dict) {
                PairParamMap pairParameters;
                std::optional<ParameterValues> defaultParameters;

                for (const auto &[key, value] : dict) {
                    const auto &pairParams = value.as<ParameterValues>();
                    if (key == "default") {
                        defaultParameters = pairParams;
                        continue;
                    }

                    auto typePair = parse_type_pair_key(key);
                    pairParameters.emplace(typePair, pairParams);
                }

                return std::make_pair(pairParameters, defaultParameters);
            });
    }

    MatcherAlternative create_lj_matcher() {
        return create_soft_interaction_matcher<LennardJonesPairData, LennardJonesInteraction>(
            "lj",
            {make_parameter_spec("epsilon", MatcherFloat{}.positive()),
             make_parameter_spec("sigma", MatcherFloat{}.positive())},
            [](const ParameterValues &parameters) {
                return LennardJonesPairData{
                    parameters.at("epsilon").as<double>(),
                    parameters.at("sigma").as<double>()
                };
            }
        );
    }

    MatcherAlternative create_wca_matcher() {
        return create_soft_interaction_matcher<RepulsiveLennardJonesPairData, RepulsiveLennardJonesInteraction>(
            "wca",
            {make_parameter_spec("epsilon", MatcherFloat{}.positive()),
             make_parameter_spec("sigma", MatcherFloat{}.positive())},
            [](const ParameterValues &parameters) {
                return RepulsiveLennardJonesPairData{
                    parameters.at("epsilon").as<double>(),
                    parameters.at("sigma").as<double>()
                };
            }
        );
    }

    MatcherAlternative create_square_inverse_core_matcher() {
        return create_soft_interaction_matcher<SquareInverseCorePairData, SquareInverseCoreInteraction>(
            "square_inverse_core",
            {make_parameter_spec("epsilon", MatcherFloat{}.positive()),
             make_parameter_spec("sigma", MatcherFloat{}.positive())},
            [](const ParameterValues &parameters) {
                return SquareInverseCorePairData{
                    parameters.at("epsilon").as<double>(),
                    parameters.at("sigma").as<double>()
                };
            }
        );
    }
}


pyon::matcher::MatcherAlternative SoftInteractionMatcher::create() {
    return MatcherDataclass("hard")
        .mapTo([](const auto &) -> std::shared_ptr<SoftInteractionFactory> { return nullptr; })
        | create_lj_matcher()
        | create_wca_matcher()
        | create_square_inverse_core_matcher();
}
