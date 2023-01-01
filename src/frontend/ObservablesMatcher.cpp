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
#include "core/observables/NematicOrder.h"
#include "core/observables/SmecticOrder.h"
#include "core/observables/BondOrder.h"
#include "core/observables/RotationMatrixDrift.h"
#include "core/observables/Temperature.h"
#include "core/observables/Pressure.h"


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
    MatcherDataclass create_nematic_order();
    MatcherDataclass create_smectic_order();
    MatcherDataclass create_bond_order();
    MatcherDataclass create_rotation_matrix_drift();
    MatcherDataclass create_temperature();
    MatcherDataclass create_pressure();


    MatcherAlternative create_observable_matcher(std::size_t maxThreads) {
        return create_number_density()
            | create_box_dimensions()
            | create_packing_fraction()
            | create_compressibility_factor()
            | create_energy_per_particle()
            | create_energy_fluctuations_per_particle()
            | create_nematic_order()
            | create_smectic_order()
            | create_bond_order()
            | create_rotation_matrix_drift()
            | create_temperature()
            | create_pressure();
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

    MatcherDataclass create_nematic_order() {
        return MatcherDataclass("nematic_order")
            .arguments({{"dump_qtensor", MatcherBoolean{}, "False"}})
            .mapTo([](const DataclassData &nematicOrder) -> ObservableData {
                auto dumpQTensor = nematicOrder["dump_qtensor"].as<bool>();
                return std::make_pair(FULL_SCOPE, std::make_shared<NematicOrder>(dumpQTensor));
            });
    }

    MatcherDataclass create_smectic_order() {
        auto singleHkl = MatcherInt{}
            .positive()
            .mapTo([](long i) {
                auto iUl = static_cast<std::size_t>(i);
                return std::array<std::size_t, 3>{iUl, iUl, iUl};
            });
        auto allHkl = MatcherArray{}
            .elementsMatch(MatcherInt{}.nonNegative().mapTo<std::size_t>())
            .size(3)
            .filter([](const ArrayData &array) {
                return std::any_of(array.begin(), array.end(), [](const Any &any) {
                    return any.as<std::size_t>() > 0;
                });
            })
            .describe("with at least one non-zero index")
            .mapToStdArray<std::size_t, 3>();
        auto maxHkl = singleHkl | allHkl;

        return MatcherDataclass("smectic_order")
            .arguments({{"max_hkl", maxHkl},
                        {"dump_tau_vector", MatcherBoolean{}, "False"},
                        {"focal_point", MatcherString{}.nonEmpty(), R"("o")"}})
            .mapTo([](const DataclassData &smecticOrder) -> ObservableData {
                auto maxHkl = smecticOrder["max_hkl"].as<std::array<std::size_t, 3>>();
                auto dumpTauVector = smecticOrder["dump_tau_vector"].as<bool>();
                auto focalPoint = smecticOrder["focal_point"].as<std::string>();
                auto observable = std::make_shared<SmecticOrder>(maxHkl, dumpTauVector, focalPoint);
                return std::make_pair(FULL_SCOPE, observable);
            });
    }

    MatcherDataclass create_bond_order() {
        auto hkl = MatcherArray{}
            .elementsMatch(MatcherInt{}.mapTo<int>())
            .size(3)
            .filter([](const ArrayData &array) {
                return std::any_of(array.begin(), array.end(), [](const Any &any) {
                    return any.as<int>() != 0;
                });
            })
            .describe("with at least one non-zero index")
            .mapToStdArray<int, 3>();

        auto singleRank = MatcherInt{}
            .greaterEquals(3)
            .mapTo([](long i) {
                return std::vector<std::size_t>{static_cast<std::size_t>(i)};
            });
        auto ranksArray = MatcherArray{}
            .elementsMatch(MatcherInt{}.greaterEquals(3).mapTo<std::size_t>())
            .nonEmpty()
            .mapToStdVector<std::size_t>();
        auto ranks = singleRank | ranksArray;

        auto namedPoint = MatcherString{}.nonEmpty();

        return MatcherDataclass("bond_order")
            .arguments({{"hkl", hkl},
                        {"ranks", ranks},
                        {"layering_point", namedPoint, R"("o")"},
                        {"focal_point", namedPoint, R"("o")"}})
            .mapTo([](const DataclassData &bondOrder) -> ObservableData {
                auto hkl = bondOrder["hkl"].as<std::array<int, 3>>();
                auto ranks = bondOrder["ranks"].as<std::vector<std::size_t>>();
                auto layeringPoint = bondOrder["layering_point"].as<std::string>();
                auto focalPoint = bondOrder["focal_point"].as<std::string>();
                auto observable = std::make_shared<BondOrder>(ranks, hkl, layeringPoint, focalPoint);
                return {FULL_SCOPE, observable};
            });
    }

    MatcherDataclass create_rotation_matrix_drift() {
        return MatcherDataclass("rotation_matrix_drift").mapTo([](const DataclassData &) -> ObservableData {
            return std::make_pair(FULL_SCOPE, std::make_shared<RotationMatrixDrift>());
        });
    }

    MatcherDataclass create_temperature() {
        return MatcherDataclass("temperature").mapTo([](const DataclassData &) -> ObservableData {
            return std::make_pair(FULL_SCOPE, std::make_shared<Temperature>());
        });
    }

    MatcherDataclass create_pressure() {
        return MatcherDataclass("pressure").mapTo([](const DataclassData &) -> ObservableData {
            return std::make_pair(FULL_SCOPE, std::make_shared<Pressure>());
        });
    }
}


MatcherArray ObservableMatcher::createObservablesMatcher(std::size_t maxThreads) {
    if (maxThreads == 0)
        maxThreads = OMP_MAXTHREADS;

    return MatcherArray{}
        .elementsMatch(create_scoped_observable_matcher(maxThreads))
        .mapToStdVector<ObservableData>();
}

MatcherArray ObservableMatcher::createBulkObservablesMatcher(std::size_t maxThreads) {
    if (maxThreads == 0)
        maxThreads = OMP_MAXTHREADS;

    return MatcherArray{}.mapToStdVector<std::shared_ptr<BulkObservable>>();
}
