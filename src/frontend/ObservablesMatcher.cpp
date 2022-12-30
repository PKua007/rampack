//
// Created by Piotr Kubala on 30/12/2022.
//

#include "ObservablesMatcher.h"
#include "utils/OMPMacros.h"
#include "core/ObservablesCollector.h"
#include "core/observables/NumberDensity.h"
#include "core/observables/BoxDimensions.h"
#include "core/observables/PackingFraction.h"
#include "core/observables/CompressibilityFactor.h"
#include "core/observables/EnergyPerParticle.h"
#include "core/observables/EnergyFluctuationsPerParticle.h"

using namespace pyon::matcher;

namespace {
    using ObservableType = ObservablesCollector::ObservableType;
    using ObservableData = std::pair<std::size_t, std::shared_ptr<Observable>>;


    constexpr auto FULL_SCOPE = ObservableType::SNAPSHOT_AVERAGING_INLINE;


    MatcherAlternative create_observable_matcher(std::size_t maxThreads);
    MatcherAlternative create_scoped_observable_matcher(std::size_t maxThreads);

    MatcherDataclass create_number_density();
    MatcherDataclass create_box_dimensions();
    MatcherDataclass create_packing_fraction();
    MatcherDataclass create_compressibility_factor();
    MatcherDataclass create_energy_per_particle();
    MatcherDataclass create_energy_fluctuations_per_particle();


    MatcherAlternative create_observable_matcher(std::size_t maxThreads) {
        return create_number_density()
            | create_box_dimensions()
            | create_packing_fraction()
            | create_compressibility_factor()
            | create_energy_per_particle()
            | create_energy_fluctuations_per_particle();
    }

    MatcherAlternative create_scoped_observable_matcher(std::size_t maxThreads) {
        auto observable = create_observable_matcher(maxThreads);
        auto snapshot = MatcherBoolean{}.mapTo([](bool enabled) -> std::size_t {
            return enabled ? static_cast<std::size_t>(ObservableType::SNAPSHOT) : 0;
        });
        auto averaging = MatcherBoolean{}.mapTo([](bool enabled) -> std::size_t {
            return enabled ? static_cast<std::size_t>(ObservableType::AVERAGING) : 0;
        });
        auto inline_ = MatcherBoolean{}.mapTo([](bool enabled) -> std::size_t {
            return enabled ? static_cast<std::size_t>(ObservableType::INLINE) : 0;
        });

        auto scoped = MatcherDataclass("scoped")
            .arguments({{"obs", observable},
                        {"snapshot", snapshot, "False"},
                        {"averaging", averaging, "False"},
                        {"inline", inline_, "False"}})
            .filter([](const DataclassData &scoped) {
                return scoped["snapshot"].as<std::size_t>() != 0
                        || scoped["averaging"].as<std::size_t>() != 0
                        || scoped["inline"].as<std::size_t>() != 0;
            })
            .describe("with at least one scope active")
            .mapTo([](const DataclassData &scoped) {
                std::size_t scope = scoped["snapshot"].as<std::size_t>()
                        | scoped["averaging"].as<std::size_t>()
                        | scoped["inline"].as<std::size_t>();
                auto observable = scoped["obs"].as<ObservableData>().second;
                return std::make_pair(scope, observable);
            });

        return observable | scoped;
    }

    MatcherDataclass create_number_density() {
        return MatcherDataclass("number_density").mapTo([](const DataclassData &) -> ObservableData {
            return std::make_pair(FULL_SCOPE, std::make_shared<NumberDensity>());
        });
    }

    MatcherDataclass create_box_dimensions() {
        return MatcherDataclass("box_dimensions").mapTo([](const DataclassData &) -> ObservableData {
            return std::make_pair(FULL_SCOPE, std::make_shared<BoxDimensions>());
        });
    }

    MatcherDataclass create_packing_fraction() {
        return MatcherDataclass("packing_fraction").mapTo([](const DataclassData &) -> ObservableData {
            return std::make_pair(FULL_SCOPE, std::make_shared<PackingFraction>());
        });
    }

    MatcherDataclass create_compressibility_factor() {
        return MatcherDataclass("compressibility_factor").mapTo([](const DataclassData &) -> ObservableData {
            return std::make_pair(FULL_SCOPE, std::make_shared<CompressibilityFactor>());
        });
    }

    MatcherDataclass create_energy_per_particle() {
        return MatcherDataclass("energy_per_particle").mapTo([](const DataclassData &) -> ObservableData {
            return std::make_pair(FULL_SCOPE, std::make_shared<EnergyPerParticle>());
        });
    }

    MatcherDataclass create_energy_fluctuations_per_particle() {
        return MatcherDataclass("energy_fluctuations_per_particle").mapTo([](const DataclassData &) -> ObservableData {
            return std::make_pair(FULL_SCOPE, std::make_shared<EnergyFluctuationsPerParticle>());
        });
    }
}


MatcherArray ObservableMatcher::createObservablesMatcher(std::size_t maxThreads) {
    if (maxThreads == 0)
        maxThreads = OMP_MAXTHREADS;

    return MatcherArray()
        .elementsMatch(create_scoped_observable_matcher(maxThreads))
        .mapToStdVector<ObservableData>();
}

MatcherArray ObservableMatcher::createBulkObservablesMatcher(std::size_t maxThreads) {
    if (maxThreads == 0)
        maxThreads = OMP_MAXTHREADS;

    return {};
}
