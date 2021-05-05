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

#include "utils/Utils.h"
#include "utils/Assertions.h"

std::unique_ptr<ObservablesCollector> ObservablesCollectorFactory::create(const std::vector<std::string> &observables) {
    auto collector = std::make_unique<ObservablesCollector>();

    std::regex typePattern(R"(^(?:inline|snapshot|averaging)(?:\/(?:inline|snapshot|averaging))*$)");

    for (auto observable : observables) {
        trim(observable);
        std::istringstream observableStream(observable);

        std::string firstToken;
        observableStream >> firstToken;
        ValidateMsg(observableStream, "Malformed observable: " + observable);

        std::string observableName;
        std::size_t observableType{};
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
            collector->addObservable(std::make_unique<NematicOrder>(), observableType);
        } else {
            throw ValidationException("Unknown observable: " + observableName);
        }
    }

    return collector;
}
