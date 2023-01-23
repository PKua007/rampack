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

#include "core/observables/trackers/FourierTracker.h"
#include "core/observables/trackers/DummyTracker.h"

#include "core/observables/correlation/PairDensityCorrelation.h"
#include "core/observables/correlation/RadialEnumerator.h"
#include "core/observables/correlation/LayerwiseRadialEnumerator.h"
#include "core/observables/correlation/PairAveragedCorrelation.h"
#include "core/observables/correlation/S110Correlation.h"
#include "core/observables/DensityHistogram.h"


using namespace pyon::matcher;

namespace {
    using ObservableType = ObservablesCollector::ObservableType;
    using ObservableData = ObservablesMatcher::ObservableData;

    struct FunctionData {
        std::string shortName;
        FourierTracker::Function function;
    };


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

    MatcherDataclass create_raw_fourier_tracker();
    std::shared_ptr<FourierTracker> do_create_fourier_tracker(const DataclassData &fourierTracker);
    MatcherDataclass create_fourier_tracker();
    MatcherDataclass create_fourier_tracker_observable();

    FunctionData create_axis_function(ShapeGeometry::Axis axis, std::size_t component);

    MatcherAlternative create_bulk_observable_matcher(std::size_t maxThreads);

    MatcherDataclass create_pair_density_correlation(std::size_t maxThreads);
    MatcherDataclass create_pair_averaged_correlation(std::size_t maxThreads);
    MatcherDataclass create_density_histogram(std::size_t maxThreads);

    MatcherDataclass create_radial();
    MatcherDataclass create_layerwise_radial();
    MatcherAlternative create_tracker();


    auto positiveWavenumbers = MatcherArray{}
        .elementsMatch(MatcherInt{}.nonNegative().mapTo<std::size_t>())
        .size(3)
        .filter([](const ArrayData &array) {
            return std::any_of(array.begin(), array.end(), [](const Any &any) {
                return any.as<std::size_t>() > 0;
            });
        })
        .describe("with at least one non-zero element")
        .mapToStdArray<std::size_t, 3>();

    auto nonzeroWavenumbers = MatcherArray{}
        .elementsMatch(MatcherInt{}.mapTo<int>())
        .size(3)
        .filter([](const ArrayData &array) {
            return std::any_of(array.begin(), array.end(), [](const Any &any) {
                return any.as<int>() != 0;
            });
        })
        .describe("with at least one non-zero element")
        .mapToStdArray<int, 3>();

    auto binning = create_radial() | create_layerwise_radial();

    auto primaryAxis = MatcherString("primary").mapTo([](const std::string&) {
        return ShapeGeometry::Axis::PRIMARY;
    });
    auto secondaryAxis = MatcherString("secondary").mapTo([](const std::string&) {
        return ShapeGeometry::Axis::SECONDARY;
    });
    auto auxiliaryAxis = MatcherString("auxiliary").mapTo([](const std::string&) {
        return ShapeGeometry::Axis::AUXILIARY;
    });
    auto shapeAxis = primaryAxis | secondaryAxis | auxiliaryAxis;


    MatcherAlternative create_observable_matcher([[maybe_unused]] std::size_t maxThreads) {
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
            | create_pressure()
            | create_fourier_tracker_observable();
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
            .mapTo([](const DataclassData &scoped) -> ObservableData {
                std::size_t scope = scoped["snapshot"].as<std::size_t>()
                        | scoped["averaging"].as<std::size_t>()
                        | scoped["inline"].as<std::size_t>();
                auto observable = scoped["obs"].as<ObservableData>().observable;
                return {scope, observable};
            });

        return observable | scoped;
    }

    MatcherDataclass create_number_density() {
        return MatcherDataclass("number_density").mapTo([](const DataclassData &) -> ObservableData {
            return {FULL_SCOPE, std::make_shared<NumberDensity>()};
        });
    }

    MatcherDataclass create_box_dimensions() {
        return MatcherDataclass("box_dimensions").mapTo([](const DataclassData &) -> ObservableData {
            return {FULL_SCOPE, std::make_shared<BoxDimensions>()};
        });
    }

    MatcherDataclass create_packing_fraction() {
        return MatcherDataclass("packing_fraction").mapTo([](const DataclassData &) -> ObservableData {
            return {FULL_SCOPE, std::make_shared<PackingFraction>()};
        });
    }

    MatcherDataclass create_compressibility_factor() {
        return MatcherDataclass("compressibility_factor").mapTo([](const DataclassData &) -> ObservableData {
            return {FULL_SCOPE, std::make_shared<CompressibilityFactor>()};
        });
    }

    MatcherDataclass create_energy_per_particle() {
        return MatcherDataclass("energy_per_particle").mapTo([](const DataclassData &) -> ObservableData {
            return {FULL_SCOPE, std::make_shared<EnergyPerParticle>()};
        });
    }

    MatcherDataclass create_energy_fluctuations_per_particle() {
        return MatcherDataclass("energy_fluctuations_per_particle").mapTo([](const DataclassData &) -> ObservableData {
            return {FULL_SCOPE, std::make_shared<EnergyFluctuationsPerParticle>()};
        });
    }

    MatcherDataclass create_nematic_order() {
        return MatcherDataclass("nematic_order")
            .arguments({{"dump_qtensor", MatcherBoolean{}, "False"}})
            .mapTo([](const DataclassData &nematicOrder) -> ObservableData {
                auto dumpQTensor = nematicOrder["dump_qtensor"].as<bool>();
                return {FULL_SCOPE, std::make_shared<NematicOrder>(dumpQTensor)};
            });
    }

    // TODO: single hkl
    MatcherDataclass create_smectic_order() {
        auto singleHkl = MatcherInt{}
            .positive()
            .mapTo([](long i) {
                auto iUl = static_cast<std::size_t>(i);
                return std::array<std::size_t, 3>{iUl, iUl, iUl};
            });
        auto allHkl = positiveWavenumbers;
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
                return {FULL_SCOPE, observable};
            });
    }

    MatcherDataclass create_bond_order() {
        auto hkl = nonzeroWavenumbers;

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
            return {FULL_SCOPE, std::make_shared<RotationMatrixDrift>()};
        });
    }

    MatcherDataclass create_temperature() {
        return MatcherDataclass("temperature").mapTo([](const DataclassData &) -> ObservableData {
            return {FULL_SCOPE, std::make_shared<Temperature>()};
        });
    }

    MatcherDataclass create_pressure() {
        return MatcherDataclass("pressure").mapTo([](const DataclassData &) -> ObservableData {
            return {FULL_SCOPE, std::make_shared<Pressure>()};
        });
    }

    // TODO: focal point
    MatcherDataclass create_raw_fourier_tracker() {
        auto wavenumbers = positiveWavenumbers;

        auto constFunction = MatcherDataclass("const")
            .mapTo([](const DataclassData&) -> FunctionData {
                return {
                    "const",
                    [](const Shape&, const ShapeTraits&) { return 1; }
                };
            });

        auto axisComp = MatcherString{}
            .anyOf({"x", "y", "z"})
            .mapTo([](const std::string &str) -> std::size_t {
                return str.front() - 'x';
            });

        auto axisFunction = MatcherDataclass("axis")
            .arguments({{"which", shapeAxis},
                        {"comp",  axisComp}})
            .mapTo([](const DataclassData &axis) -> FunctionData {
                auto whichAxis = axis["which"].as<ShapeGeometry::Axis>();
                auto axisComp = axis["comp"].as<std::size_t>();
                return create_axis_function(whichAxis, axisComp);
            });

        auto function = constFunction | axisFunction;

        return MatcherDataclass("fourier_tracker")
            .arguments({{"wavenumbers", wavenumbers},
                        {"function", function}});
    }

    std::shared_ptr<FourierTracker> do_create_fourier_tracker(const DataclassData &fourierTracker) {
        auto wavenumbers = fourierTracker["wavenumbers"].as<std::array<std::size_t, 3>>();
        auto function = fourierTracker["function"].as<FunctionData>();
        return std::make_shared<FourierTracker>(wavenumbers, function.function, function.shortName);
    }

    MatcherDataclass create_fourier_tracker() {
        return create_raw_fourier_tracker()
            .mapTo([](const DataclassData &fourierTracker) -> std::shared_ptr<GoldstoneTracker> {
                return do_create_fourier_tracker(fourierTracker);
            });
    }

    MatcherDataclass create_fourier_tracker_observable() {
        return create_raw_fourier_tracker()
            .mapTo([](const DataclassData &fourierTracker) -> ObservableData {
                return {FULL_SCOPE, do_create_fourier_tracker(fourierTracker)};
            });
    }

    FunctionData create_axis_function(ShapeGeometry::Axis axis, std::size_t component) {
        using Axis = ShapeGeometry::Axis;

        FourierTracker::Function function;
        std::string shortName;

        switch (axis) {
            case Axis::PRIMARY:
                shortName = "pa_";
                function = [component](const Shape &shape, const ShapeTraits &traits) {
                    return traits.getGeometry().getPrimaryAxis(shape)[component];
                };
                break;

            case Axis::SECONDARY:
                shortName = "sa_";
                function = [component](const Shape &shape, const ShapeTraits &traits) {
                    return traits.getGeometry().getSecondaryAxis(shape)[component];
                };
                break;

            case Axis::AUXILIARY:
                shortName = "aa_";
                function = [component](const Shape &shape, const ShapeTraits &traits) {
                    return traits.getGeometry().getAuxiliaryAxis(shape)[component];
                };
                break;
        }

        auto compName = static_cast<char>('x' + component);
        shortName += std::string{compName};

        return {shortName, function};
    }

    MatcherAlternative create_bulk_observable_matcher(std::size_t maxThreads) {
        return create_pair_density_correlation(maxThreads)
            | create_pair_averaged_correlation(maxThreads)
            | create_density_histogram(maxThreads);
    }

    MatcherDataclass create_pair_density_correlation(std::size_t maxThreads) {
        return MatcherDataclass("pair_density_correlation")
            .arguments({{"max_r", MatcherFloat{}.positive()},
                        {"n_bins", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"binning", binning}})
            .mapTo([maxThreads](const DataclassData &pairDensityCorrelation) -> std::shared_ptr<BulkObservable> {
                auto maxR = pairDensityCorrelation["max_r"].as<double>();
                auto nBins = pairDensityCorrelation["n_bins"].as<std::size_t>();
                auto binning = pairDensityCorrelation["binning"].as<std::shared_ptr<PairEnumerator>>();
                return std::make_shared<PairDensityCorrelation>(binning, maxR, nBins, maxThreads);
            });
    }

    MatcherDataclass create_pair_averaged_correlation(std::size_t maxThreads) {
        auto function = MatcherDataclass("S110")
            .arguments({{"axis", shapeAxis}})
            .mapTo([](const DataclassData &s110) -> std::shared_ptr<CorrelationFunction> {
                auto axis = s110["axis"].as<ShapeGeometry::Axis>();
                return std::make_shared<S110Correlation>(axis);
            });

        return MatcherDataclass("pair_averaged_correlation")
            .arguments({{"max_r", MatcherFloat{}.positive()},
                        {"n_bins", MatcherInt{}.greaterEquals(2).mapTo<std::size_t>()},
                        {"binning", binning},
                        {"function", function}})
            .mapTo([maxThreads](const DataclassData &pairAveragedCorrelation) -> std::shared_ptr<BulkObservable> {
                auto maxR = pairAveragedCorrelation["max_r"].as<double>();
                auto nBins = pairAveragedCorrelation["n_bins"].as<std::size_t>();
                auto binning = pairAveragedCorrelation["binning"].as<std::shared_ptr<PairEnumerator>>();
                auto function = pairAveragedCorrelation["function"].as<std::shared_ptr<CorrelationFunction>>();
                return std::make_shared<PairAveragedCorrelation>(binning, function, maxR, nBins, maxThreads);
            });
    }

    MatcherDataclass create_density_histogram(std::size_t maxThreads) {
        auto nBinsInt = MatcherInt{}.greaterEquals(2).mapTo<std::size_t>();
        auto nBinsNone = MatcherNone{}.mapTo([]() -> std::size_t { return 1; });
        auto nBins = nBinsInt | nBinsNone;

        auto tracker = create_tracker();

        return MatcherDataclass("density_histogram")
            .arguments({{"n_bins_x", nBins, "None"},
                        {"n_bins_y", nBins, "None"},
                        {"n_bins_z", nBins, "None"},
                        {"tracker", tracker, "None"}})
            .filter([](const DataclassData &densityHistoram) {
                return densityHistoram["n_bins_x"].as<std::size_t>() != 1
                    || densityHistoram["n_bins_y"].as<std::size_t>() != 1
                    || densityHistoram["n_bins_z"].as<std::size_t>() != 1;
            })
            .describe("with at least 2 bins in at least one direction")
            .mapTo([maxThreads](const DataclassData &densityHistogram) -> std::shared_ptr<BulkObservable> {
                std::array<std::size_t, 3> nBins{
                    densityHistogram["n_bins_x"].as<std::size_t>(),
                    densityHistogram["n_bins_y"].as<std::size_t>(),
                    densityHistogram["n_bins_z"].as<std::size_t>()
                };
                auto tracker = densityHistogram["tracker"].as<std::shared_ptr<GoldstoneTracker>>();
                return std::make_shared<DensityHistogram>(nBins, tracker, maxThreads);
            });
    }

    MatcherDataclass create_radial() {
        return MatcherDataclass("radial")
            .arguments({{"focal_point", MatcherString{}.nonEmpty(), R"("o")"}})
            .mapTo([](const DataclassData &radial) -> std::shared_ptr<PairEnumerator> {
                auto focalPoint = radial["focal_point"].as<std::string>();
                return std::make_shared<RadialEnumerator>(focalPoint);
            });
    }

    MatcherDataclass create_layerwise_radial() {
        return MatcherDataclass("layerwise_radial")
            .arguments({{"hkl", nonzeroWavenumbers},
                        {"focal_point", MatcherString{}.nonEmpty(), R"("o")"}})
            .mapTo([](const DataclassData &layerwiseRadial) -> std::shared_ptr<PairEnumerator> {
                auto hkl = layerwiseRadial["hkl"].as<std::array<int, 3>>();
                auto focalPoint = layerwiseRadial["focal_point"].as<std::string>();
                return std::make_shared<LayerwiseRadialEnumerator>(hkl, focalPoint);
            });
    }

    MatcherAlternative create_tracker() {
        auto noneTracker = MatcherNone{}.mapTo([]() -> std::shared_ptr<GoldstoneTracker> {
            return std::make_shared<DummyTracker>();
        });

        return noneTracker | create_fourier_tracker();
    }
}


MatcherAlternative ObservablesMatcher::createObservablesMatcher(std::size_t maxThreads) {
    if (maxThreads == 0)
        maxThreads = OMP_MAXTHREADS;

    return create_scoped_observable_matcher(maxThreads);
}

MatcherAlternative ObservablesMatcher::createBulkObservablesMatcher(std::size_t maxThreads) {
    if (maxThreads == 0)
        maxThreads = OMP_MAXTHREADS;

    return create_bulk_observable_matcher(maxThreads);
}
