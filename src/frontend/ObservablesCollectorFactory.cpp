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
            std::vector<std::string> tokens;
            std::copy(std::istream_iterator<std::string>(observableStream), std::istream_iterator<std::string>{},
                      std::back_inserter(tokens));

            bool dumpTauVector{};
            std::string dumpTauVectorString;
            switch (tokens.size()) {
                case 0:
                case 3:
                    dumpTauVector = false;
                    break;

                case 1:
                case 4:
                    dumpTauVectorString = tokens.back();
                    if (dumpTauVectorString != "dumpTauVector") {
                        throw ValidationException("Malformed smectic order, usage: smecticOrder "
                                                  "([max_k_x] [max_k_y] [max_k_z]) (dumpTauVector)");
                    }
                    dumpTauVector = true;
                    break;

                default:
                    throw ValidationException("Malformed smectic order, usage: smecticOrder "
                                              "([max_k_x] [max_k_y] [max_k_z]) (dumpTauVector)");
                    break;
            }

            if (tokens.size() >= 3) {
                std::array<int, 3> kTauRanges{std::stoi(tokens[0]), std::stoi(tokens[1]), std::stoi(tokens[2])};
                bool anyNonzero = std::any_of(kTauRanges.begin(), kTauRanges.end(), [](int i) { return i != 0; });
                bool allNonNegative = std::all_of(kTauRanges.begin(), kTauRanges.end(), [](int i) { return i >= 0; });
                ValidateMsg(anyNonzero && allNonNegative, "All tau ranges must be nonzero and some must be positive");
                collector->addObservable(std::make_unique<SmecticOrder>(kTauRanges, dumpTauVector), observableType);
            } else {
                collector->addObservable(std::make_unique<SmecticOrder>(dumpTauVector), observableType);
            }
        } else if (observableName == "bondOrder") {
            std::string millerIndicesString;
            observableStream >> millerIndicesString;
            auto millerIndicesExploded = explode(millerIndicesString, '.');
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


            std::vector<std::size_t> ranks;
            std::copy(std::istream_iterator<std::size_t>(observableStream), std::istream_iterator<std::size_t>{},
                      std::back_inserter(ranks));
            std::sort(ranks.begin(), ranks.end());
            bool allRanksOk = std::all_of(ranks.begin(), ranks.end(), [](int rank) { return rank >= 2; });
            ValidateMsg(allRanksOk, "Bond order: some ranks are not >= 2");
            bool allUnique = std::adjacent_find(ranks.begin(), ranks.end()) == ranks.end();
            ValidateMsg(allUnique, "Bond order: some ranks are repeated");

            collector->addObservable(std::make_unique<BondOrder>(ranks, millerIndices), observableType);
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
