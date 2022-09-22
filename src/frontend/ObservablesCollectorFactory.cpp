//
// Created by Piotr Kubala on 25/03/2021.
//

#include <sstream>
#include <regex>

#include "ObservablesCollectorFactory.h"

#include "core/observables/NumberDensity.h"
#include "core/observables/BoxDimensions.h"
#include "core/observables/PackingFraction.h"
#include "core/observables/CompressibilityFactor.h"
#include "core/observables/EnergyPerParticle.h"
#include "core/observables/EnergyFluctuationsPerParticle.h"
#include "core/observables/NematicOrder.h"
#include "core/observables/SmecticOrder.h"
#include "core/observables/BondOrder.h"
#include "core/observables/RotationMatrixDrift.h"
#include "core/observables/Temperature.h"
#include "core/observables/Pressure.h"
#include "core/observables/correlation/RadialEnumerator.h"
#include "core/observables/correlation/LayerwiseRadialEnumerator.h"
#include "core/observables/correlation/PairDensityCorrelation.h"
#include "core/observables/correlation/PairAveragedCorrelation.h"
#include "core/observables/correlation/S110Correlation.h"

#include "utils/Utils.h"
#include "utils/Assertions.h"
#include "ParseUtils.h"


#define SMECTIC_ORDER_USAGE "Malformed smectic order, usage:\n" \
                            "OLD SYNTAX: smecticOrder ([max_n x] [max_n y] [max_n z]) (dumpTauVector)\n" \
                            "NEW SYNTAX: smecticOrder (max_n {[x=y=z] | [x] [y] [z]}) (dumpTauVector) " \
                            "(focalPoint [point name])"

#define BOND_ORDER_USAGE "Malformed bond order, usage:\n" \
                         "OLD SYNTAX: bondOrder [nx].[ny].[nz] [rank 1] [rank 2] ...\n" \
                         "NEW SYNTAX: bondOrder millerIdx [nx].[ny].[nz] ranks [rank 1] [rank 2] ... " \
                         "(layeringPoint [point name]) (bondOrderPoint [point name])"

#define BINNING_SPEC_ALTERNATIVES "1. radial [focal point = cm]\n" \
                                  "2. layerwiseRadial [nx].[ny].[nz] [focal point = cm]"

#define BINNING_SPEC_USAGE "Malformed binning specifications. Available alternatives:\n" \
                           BINNING_SPEC_ALTERNATIVES

#define PAIR_DENSITY_CORRELATION_USAGE "Malformed pair density correlation, usage:\n" \
                                       "pairDensityCorrelation [max distance] [num bins] [binning specification] \n" \
                                       "[binning specification] :=\n"\
                                       BINNING_SPEC_ALTERNATIVES

#define CORR_FUN_ALTERNATIVES "1. S110 (primary|secondary)"

#define CORR_FUN_USAGE "Malformed correlation function. Available alternatives:\n" \
                       CORR_FUN_ALTERNATIVES

#define PAIR_AVERAGED_CORRELATION_USAGE "Malformed pair averaged correlation, usage:\n" \
                                        "pairAveragedCorrelation [max distance] [num bins] [correlation function] " \
                                        "[binning specification] \n" \
                                        "[binning specification] :=\n" \
                                        BINNING_SPEC_ALTERNATIVES "\n" \
                                        "[correlation function] :=\n" \
                                        CORR_FUN_ALTERNATIVES

namespace {
    auto parse_observable_name_and_type(std::istringstream &observableStream, const std::regex &typePattern) {
        std::string observable = observableStream.str();

        std::string observableName;
        std::size_t observableType{};

        std::string firstToken;
        observableStream >> firstToken;
        ValidateMsg(observableStream, "Malformed observable: " + observable);
        if (std::regex_match(firstToken, typePattern)) {
            auto types = explode(firstToken, '/');
            for (const auto &type : types) {
                if (type == "inline")
                    observableType |= ObservablesCollector::INLINE;
                else if (type == "snapshot")
                    observableType |= ObservablesCollector::SNAPSHOT;
                else if (type == "averaging")
                    observableType |= ObservablesCollector::AVERAGING;
                else
                    throw AssertionException("observable type: " + type);
            }
            observableStream >> observableName;
            ValidateMsg(observableStream, "Malformed observable: " + observable);
        } else {
            observableName = firstToken;
            using OC = ObservablesCollector;
            observableType = OC::SNAPSHOT | OC::AVERAGING | OC::INLINE;
        }

        return std::make_pair(observableName, observableType);
    }

    std::map<std::string, std::string> parse_smectic_order_old_syntax(const std::vector<std::string> &tokens) {
        switch (tokens.size()) {
            case 0: case 1: case 3: case 4:
                break;
            default:
                throw ValidationException(SMECTIC_ORDER_USAGE);
        }

        std::map<std::string, std::string> fieldMap;

        if (tokens.size() == 1 || tokens.size() == 4) {
            if (tokens.back() != "dumpTauVector")
                throw ValidationException(SMECTIC_ORDER_USAGE);
            fieldMap["dumpTauVector"] = "";
        }

        if (tokens.size() == 3 || tokens.size() == 4)
            fieldMap["max_n"] = tokens[0] + " " + tokens[1] + " " + tokens[2];

        return fieldMap;
    }

    std::array<int, 3> parse_smectic_order_max_n(const std::map<std::string, std::string> &fieldMap) {
        if (!(fieldMap.find("max_n") != fieldMap.end()))
            return {5, 5, 5};

        auto maxNToneks = ParseUtils::tokenize<int>(fieldMap.at("max_n"));
        ValidateMsg(maxNToneks.size() == 1 || maxNToneks.size() == 3, "smectic order: max_n should be 1 or 3 ints");
        std::array<int, 3> maxN{};
        if (maxNToneks.size() == 1)
            maxN = {maxNToneks[0], maxNToneks[0], maxNToneks[0]};
        else // maxNToneks.size() == 3
            maxN = {maxNToneks[0], maxNToneks[1], maxNToneks[2]};

        bool anyNonzero = std::any_of(maxN.begin(), maxN.end(), [](int i) { return i != 0; });
        bool allNonNegative = std::all_of(maxN.begin(), maxN.end(), [](int i) { return i >= 0; });
        ValidateMsg(anyNonzero && allNonNegative, "All n ranges must be nonzero and some must be positive");

        return maxN;
    }

    std::unique_ptr<SmecticOrder> parse_smectic_order(std::istringstream &observableStream) {
        std::vector<std::string> tokens = ParseUtils::tokenize<std::string>(observableStream);
        auto fieldMap = ParseUtils::parseFields({"", "max_n", "dumpTauVector", "focalPoint"}, tokens);

        if (fieldMap.find("") != fieldMap.end())
            fieldMap = parse_smectic_order_old_syntax(tokens);

        std::array<int, 3> maxN = parse_smectic_order_max_n(fieldMap);

        bool dumpTauVector = false;
        if (fieldMap.find("dumpTauVector") != fieldMap.end()) {
            ValidateMsg(fieldMap["dumpTauVector"].empty(), SMECTIC_ORDER_USAGE);
            dumpTauVector = true;
        }

        std::string focalPoint = "cm";
        if (fieldMap.find("focalPoint") != fieldMap.end())
            focalPoint = fieldMap["focalPoint"];

        return std::make_unique<SmecticOrder>(maxN, dumpTauVector, focalPoint);
    }

    std::map<std::string, std::string> parse_bond_order_old_syntax(const std::vector<std::string> &tokens) {
        ValidateMsg(tokens.size() >= 2, BOND_ORDER_USAGE);

        std::map<std::string, std::string> fieldMap;
        fieldMap["millerIdx"] = tokens[0];
        auto spaceImplode = [](const auto &ranksStr, const auto &nextRankStr) { return ranksStr + " " + nextRankStr; };
        fieldMap["ranks"] = std::accumulate(std::next(tokens.begin()), tokens.end(), std::string{}, spaceImplode);
        return fieldMap;
    }

    std::array<int, 3> parse_miller_indices(const std::string &millerString) {
        auto millerIndicesExploded = explode(millerString, '.');
        ValidateMsg(millerIndicesExploded.size() == 3, "Malformed Miller indices; format: nx.ny.nz");
        std::array<int, 3> millerIndices{};
        auto converter = [](const std::string &wavenumberString) {
            try {
                return std::stoi(wavenumberString);
            } catch (std::logic_error &e) {
                throw ValidationException("Malformed Miller indices; format: nx.ny.nz");
            }
        };
        std::transform(millerIndicesExploded.begin(), millerIndicesExploded.end(), millerIndices.begin(),
                       converter);
        bool anyNonzero = std::any_of(millerIndices.begin(), millerIndices.end(), [](int i) { return i != 0; });
        ValidateMsg(anyNonzero, "All Miller indices are equal 0");
        return millerIndices;
    }

    std::vector<size_t> parse_bond_order_ranks(const std::map<std::string, std::string> &fieldMap) {
        std::vector<std::size_t> ranks = ParseUtils::tokenize<std::size_t>(fieldMap.at("ranks"));
        std::sort(ranks.begin(), ranks.end());
        bool allRanksOk = std::all_of(ranks.begin(), ranks.end(), [](int rank) { return rank >= 2; });
        ValidateMsg(allRanksOk, "Bond order: some ranks are not >= 2");
        bool allUnique = std::adjacent_find(ranks.begin(), ranks.end()) == ranks.end();
        ValidateMsg(allUnique, "Bond order: some ranks are repeated");
        return ranks;
    }

    std::unique_ptr<BondOrder> parse_bond_order(std::istringstream &observableStream) {
        std::vector<std::string> tokens = ParseUtils::tokenize<std::string>(observableStream);
        auto fieldMap = ParseUtils::parseFields({"", "millerIdx", "ranks", "layeringPoint", "bondOrderPoint"}, tokens);

        if (fieldMap.find("") != fieldMap.end())
            fieldMap = parse_bond_order_old_syntax(tokens);

        ValidateMsg(fieldMap.find("millerIdx") != fieldMap.end() && fieldMap.find("ranks") != fieldMap.end(),
                    BOND_ORDER_USAGE);

        std::array<int, 3> millerIndices = parse_miller_indices(fieldMap.at("millerIdx"));
        std::vector<size_t> ranks = parse_bond_order_ranks(fieldMap);

        std::string layeringPoint = "cm";
        if (fieldMap.find("layeringPoint") != fieldMap.end())
            layeringPoint = fieldMap["layeringPoint"];

        std::string bondOrderPoint = "cm";
        if (fieldMap.find("bondOrderPoint") != fieldMap.end())
            bondOrderPoint = fieldMap["bondOrderPoint"];

        return std::make_unique<BondOrder>(ranks, millerIndices, layeringPoint, bondOrderPoint);
    }

    bool is_anything_left(std::istringstream &observableStream) {
        if (!observableStream)
            return false;
        observableStream >> std::ws;
        return observableStream.good();
    }

    std::unique_ptr<PairEnumerator> parse_pair_enumerator(std::istringstream &observableStream) {
        std::string enumeratorName;
        observableStream >> enumeratorName;
        ValidateMsg(observableStream, BINNING_SPEC_USAGE);

        if (enumeratorName == "radial") {
            std::string focalPoint = "cm";
            if (is_anything_left(observableStream))
                observableStream >> focalPoint;
            ValidateMsg(!observableStream.fail(), BINNING_SPEC_USAGE);

            return std::make_unique<RadialEnumerator>(focalPoint);
        } else if (enumeratorName == "layerwiseRadial") {
            std::string millerString;
            observableStream >> millerString;
            ValidateMsg(observableStream, BINNING_SPEC_USAGE);
            auto millerIndices = parse_miller_indices(millerString);

            std::string focalPoint = "cm";
            if (is_anything_left(observableStream))
                observableStream >> focalPoint;
            ValidateMsg(!observableStream.fail(), BINNING_SPEC_USAGE);

            return std::make_unique<LayerwiseRadialEnumerator>(millerIndices, focalPoint);
        } else {
            throw ValidationException("Unknown binning specification: " + enumeratorName);
        }
    }

    std::unique_ptr<CorrelationFunction> parse_correlation_function(std::istringstream &observableStream) {
        std::string name;
        observableStream >> name;
        ValidateMsg(observableStream, CORR_FUN_USAGE);

        if (name == "S110") {
            std::string axisName;
            observableStream >> axisName;
            ValidateMsg(observableStream, CORR_FUN_USAGE);
            if (axisName == "primary")
                return std::make_unique<S110Correlation>(S110Correlation::Axis::PRIMARY_AXIS);
            else if (axisName == "secondary")
                return std::make_unique<S110Correlation>(S110Correlation::Axis::SECONDARY_AXIS);
            else
                throw ValidationException(CORR_FUN_USAGE);
        } else {
            throw ValidationException("Unknown correlation function: " + name);
        }
    }
    
    void parse_observables(const std::vector<std::string> &observables, ObservablesCollector &collector) {
        std::regex typePattern(R"(^(?:inline|snapshot|averaging)(?:\/(?:inline|snapshot|averaging))*$)");

        for (auto observable : observables) {
            trim(observable);
            std::istringstream observableStream(observable);

            auto [observableName, observableType] = parse_observable_name_and_type(observableStream, typePattern);

            if (observableName == "numberDensity") {
                collector.addObservable(std::make_unique<NumberDensity>(), observableType);
            } else if (observableName == "boxDimensions") {
                collector.addObservable(std::make_unique<BoxDimensions>(), observableType);
            } else if (observableName == "packingFraction") {
                collector.addObservable(std::make_unique<PackingFraction>(), observableType);
            } else if (observableName == "compressibilityFactor") {
                collector.addObservable(std::make_unique<CompressibilityFactor>(), observableType);
            } else if (observableName == "energyPerParticle") {
                collector.addObservable(std::make_unique<EnergyPerParticle>(), observableType);
            } else if (observableName == "energyFluctuationsPerParticle") {
                collector.addObservable(std::make_unique<EnergyFluctuationsPerParticle>(), observableType);
            } else if (observableName == "nematicOrder") {
                observableStream >> std::ws;
                if (observableStream.eof()) {
                    collector.addObservable(std::make_unique<NematicOrder>(), observableType);
                    continue;
                }

                std::string QTensorString;
                observableStream >> QTensorString;
                ValidateMsg(QTensorString == "dumpQTensor", "Malformed nematic order, usage: nematicOrder (dumpQTensor)");
                collector.addObservable(std::make_unique<NematicOrder>(true), observableType);
            } else if (observableName == "smecticOrder") {
                collector.addObservable(parse_smectic_order(observableStream), observableType);
            } else if (observableName == "bondOrder") {
                collector.addObservable(parse_bond_order(observableStream), observableType);
            } else if (observableName == "rotationMatrixDrift") {
                collector.addObservable(std::make_unique<RotationMatrixDrift>(), observableType);
            } else if (observableName == "temperature") {
                collector.addObservable(std::make_unique<Temperature>(), observableType);
            } else if (observableName == "pressure") {
                collector.addObservable(std::make_unique<Pressure>(), observableType);
            } else {
                throw ValidationException("Unknown observable: " + observableName);
            }
        }
    }

    void parse_bulk_observables(const std::vector<std::string> &bulkObservables, size_t maxThreads,
                                ObservablesCollector &collector)
    {
        for (const auto &bulkObservable : bulkObservables) {
            std::istringstream observableStream(bulkObservable);
            std::string observableName;
            observableStream >> observableName;
            ValidateMsg(observableStream, "Malformed bulk observable, usage: [observable name] (parameters)");

            if (observableName == "pairDensityCorrelation") {
                double maxDistance{};
                std::size_t numBins{};
                observableStream >> maxDistance >> numBins;
                ValidateMsg(observableStream, PAIR_DENSITY_CORRELATION_USAGE);
                Validate(maxDistance > 0);
                Validate(numBins >= 2);

                auto pairEnumerator = parse_pair_enumerator(observableStream);
                auto rhoCorr = std::make_unique<PairDensityCorrelation>(
                        std::move(pairEnumerator), maxDistance, numBins, maxThreads
                );
                collector.addBulkObservable(std::move(rhoCorr));
            } else if (observableName == "pairAveragedCorrelation") {
                double maxDistance{};
                std::size_t numBins{};
                observableStream >> maxDistance >> numBins;
                ValidateMsg(observableStream, PAIR_AVERAGED_CORRELATION_USAGE);
                Validate(maxDistance > 0);
                Validate(numBins >= 2);

                auto correlationFunction = parse_correlation_function(observableStream);
                auto pairEnumerator = parse_pair_enumerator(observableStream);
                auto avgCorr = std::make_unique<PairAveragedCorrelation>(
                        std::move(pairEnumerator), std::move(correlationFunction), maxDistance, numBins, maxThreads
                );
                collector.addBulkObservable(std::move(avgCorr));
            } else {
                throw ValidationException("Unknown observable: " + observableName);
            }
        }
    }
}

std::unique_ptr<ObservablesCollector>
ObservablesCollectorFactory::create(const std::vector<std::string> &observables,
                                    const std::vector<std::string> &bulkObservables, std::size_t maxThreads)
{
    auto collector = std::make_unique<ObservablesCollector>();
    parse_observables(observables, *collector);
    parse_bulk_observables(bulkObservables, maxThreads, *collector);
    return collector;
}
