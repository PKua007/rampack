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
            fieldMap["max_k"] = tokens[0] + " " + tokens[1] + " " + tokens[2];

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
        else // maxKToneks.size() == 3
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

    std::array<int, 3> parse_bond_order_miller_indices(const std::map<std::string, std::string> &fieldMap) {
        auto millerIndicesExploded = explode(fieldMap.at("millerIdx"), '.');
        ValidateMsg(millerIndicesExploded.size() == 3, "Malformed bond order Miller indices; format: nx.ny.nz");
        std::array<int, 3> millerIndices{};
        auto converter = [](const std::string &wavenumberString) {
            try {
                return std::stoi(wavenumberString);
            } catch (std::logic_error &e) {
                throw ValidationException("Malformed bond order Miller indices; format: nx.ny.nz");
            }
        };
        std::transform(millerIndicesExploded.begin(), millerIndicesExploded.end(), millerIndices.begin(),
                       converter);
        bool anyNonzero = std::any_of(millerIndices.begin(), millerIndices.end(), [](int i) { return i != 0; });
        ValidateMsg(anyNonzero, "Bond order: all Miller indices are equal 0");
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

        std::array<int, 3> millerIndices = parse_bond_order_miller_indices(fieldMap);
        std::vector<size_t> ranks = parse_bond_order_ranks(fieldMap);

        std::string layeringPoint = "cm";
        if (fieldMap.find("layeringPoint") != fieldMap.end())
            layeringPoint = fieldMap["layeringPoint"];

        std::string bondOrderPoint = "cm";
        if (fieldMap.find("bondOrderPoint") != fieldMap.end())
            bondOrderPoint = fieldMap["bondOrderPoint"];

        return std::make_unique<BondOrder>(ranks, millerIndices, layeringPoint, bondOrderPoint);
    }
}

std::unique_ptr<ObservablesCollector> ObservablesCollectorFactory::create(const std::vector<std::string> &observables) {
    auto collector = std::make_unique<ObservablesCollector>();

    std::regex typePattern(R"(^(?:inline|snapshot|averaging)(?:\/(?:inline|snapshot|averaging))*$)");

    for (auto observable : observables) {
        trim(observable);
        std::istringstream observableStream(observable);

        auto [observableName, observableType] = parse_observable_name_and_type(observableStream, typePattern);

        if (observableName == "numberDensity") {
            collector->addObservable(std::make_unique<NumberDensity>(), observableType);
        } else if (observableName == "boxDimensions") {
            collector->addObservable(std::make_unique<BoxDimensions>(), observableType);
        } else if (observableName == "packingFraction") {
            collector->addObservable(std::make_unique<PackingFraction>(), observableType);
        } else if (observableName == "compressibilityFactor") {
            collector->addObservable(std::make_unique<CompressibilityFactor>(), observableType);
        } else if (observableName == "energyPerParticle") {
            collector->addObservable(std::make_unique<EnergyPerParticle>(), observableType);
        } else if (observableName == "energyFluctuationsPerParticle") {
            collector->addObservable(std::make_unique<EnergyFluctuationsPerParticle>(), observableType);
        } else if (observableName == "nematicOrder") {
            observableStream >> std::ws;
            if (observableStream.eof()) {
                collector->addObservable(std::make_unique<NematicOrder>(), observableType);
                continue;
            }

            std::string QTensorString;
            observableStream >> QTensorString;
            ValidateMsg(QTensorString == "dumpQTensor", "Malformed nematic order, usage: nematicOrder (dumpQTensor)");
            collector->addObservable(std::make_unique<NematicOrder>(true), observableType);
        } else if (observableName == "smecticOrder") {
            collector->addObservable(parse_smectic_order(observableStream), observableType);
        } else if (observableName == "bondOrder") {
            collector->addObservable(parse_bond_order(observableStream), observableType);
        } else if (observableName == "rotationMatrixDrift") {
            collector->addObservable(std::make_unique<RotationMatrixDrift>(), observableType);
        } else if (observableName == "temperature") {
            collector->addObservable(std::make_unique<Temperature>(), observableType);
        } else if (observableName == "pressure") {
            collector->addObservable(std::make_unique<Pressure>(), observableType);
        } else {
            throw ValidationException("Unknown observable: " + observableName);
        }
    }

    return collector;
}
