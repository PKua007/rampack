//
// Created by Piotr Kubala on 25/03/2021.
//

#include <sstream>

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

    for (auto observable : observables) {
        trim(observable);
        bool isInline = false;
        std::istringstream observableStream(observable);
        if (startsWith(observable, "inline")) {
            std::string inlineString;
            observableStream >> inlineString;
            ValidateMsg(observableStream && inlineString == "inline", "Malformed observable: " + observable);
            isInline = true;
        }

        std::string observableName;
        observableStream >> observableName;
        ValidateMsg(observableStream, "Malformed observable: " + observable);

        if (observableName == "numberDensity") {
            collector->addObservable(std::make_unique<NumberDensity>(), isInline);
        } else if (observableName == "boxDimensions") {
            collector->addObservable(std::make_unique<BoxDimensions>(), isInline);
        } else if (observableName == "packingFraction") {
            collector->addObservable(std::make_unique<PackingFraction>(), isInline);
        } else if (observableName == "compressibilityFactor") {
            collector->addObservable(std::make_unique<CompressibilityFactor>(), isInline);
        } else if (observableName == "energyPerParticle") {
            collector->addObservable(std::make_unique<EnergyPerParticle>(), isInline);
        } else if (observableName == "energyFluctuationsPerParticle") {
            collector->addObservable(std::make_unique<EnergyFluctuationsPerParticle>(), isInline);
        } else if (observableName == "nematicOrder") {
            collector->addObservable(std::make_unique<NematicOrder>(), isInline);
        } else {
            throw ValidationException("Unknown observable: " + observable);
        }
    }

    return collector;
}
