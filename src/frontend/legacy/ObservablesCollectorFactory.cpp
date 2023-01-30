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
#include "core/observables/DensityHistogram.h"
#include "core/observables/correlation/RadialEnumerator.h"
#include "core/observables/correlation/LayerwiseRadialEnumerator.h"
#include "core/observables/correlation/PairDensityCorrelation.h"
#include "core/observables/correlation/PairAveragedCorrelation.h"
#include "core/observables/correlation/S110Correlation.h"
#include "core/observables/trackers/FourierTracker.h"

#include "utils/Utils.h"
#include "utils/Exceptions.h"
#include "utils/ParseUtils.h"


#define SMECTIC_ORDER_USAGE "Malformed smectic order, usage:\n" \
                            "OLD SYNTAX: smecticOrder ([max_n x] [max_n y] [max_n z]) (dumpTauVector)\n" \
                            "NEW SYNTAX: smecticOrder (max_n {[x=y=z] | [x] [y] [z]}) (dumpTauVector) " \
                            "(focalPoint [point name])"

#define BOND_ORDER_USAGE "Malformed bond order, usage:\n" \
                         "OLD SYNTAX: bondOrder [nx].[ny].[nz] [rank 1] [rank 2] ...\n" \
                         "NEW SYNTAX: bondOrder millerIdx [nx].[ny].[nz] ranks [rank 1] [rank 2] ... " \
                         "(layeringPoint [point name]) (bondOrderPoint [point name])"

#define BINNING_SPEC_ALTERNATIVES "1. radial [focal point = o]\n" \
                                  "2. layerwiseRadial [nx].[ny].[nz] [focal point = o]"

#define BINNING_SPEC_USAGE "Malformed binning specifications. Available alternatives:\n" \
                           BINNING_SPEC_ALTERNATIVES

#define PAIR_DENSITY_CORRELATION_USAGE "Malformed pair density correlation, usage:\n" \
                                       "pairDensityCorrelation [max distance] [num bins] [binning specification] \n" \
                                       "[binning specification] :=\n"\
                                       BINNING_SPEC_ALTERNATIVES

#define CORR_FUN_ALTERNATIVES "1. S110 (primary|secondary|auxiliary)"

#define CORR_FUN_USAGE "Malformed correlation function. Available alternatives:\n" \
                       CORR_FUN_ALTERNATIVES

#define PAIR_AVERAGED_CORRELATION_USAGE "Malformed pair averaged correlation, usage:\n" \
                                        "pairAveragedCorrelation [max distance] [num bins] [correlation function] " \
                                        "[binning specification] \n" \
                                        "[binning specification] :=\n" \
                                        BINNING_SPEC_ALTERNATIVES "\n" \
                                        "[correlation function] :=\n" \
                                        CORR_FUN_ALTERNATIVES

#define FOURIER_TRACKER_USAGE "Malformed Fourier tracker, usage:\n" \
                              "fourierTracker [wavenumber x] [... y] [... z] [function]\n" \
                              "[function] :=\n" \
                              "1. const\n" \
                              "2. (primaryAxis|secondaryAxis|auxiliaryAxis) (x|y|z)"

#define DENSITY_HISTOGRAM_USAGE "Malformed density histogram, usage:\n" \
                                "densityHistogram n_bins [number of bins x] [... y] [... z] " \
                                "(tracker [tracker name] ([tracker parameters]))\n" \
                                "[tracker name] :=\n" \
                                "1. fourierTracker"

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
                    AssertThrow("observable type: " + type);
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
                throw InputError(SMECTIC_ORDER_USAGE);
        }

        std::map<std::string, std::string> fieldMap;

        if (tokens.size() == 1 || tokens.size() == 4) {
            if (tokens.back() != "dumpTauVector")
                throw InputError(SMECTIC_ORDER_USAGE);
            fieldMap["dumpTauVector"] = "";
        }

        if (tokens.size() == 3 || tokens.size() == 4)
            fieldMap["max_n"] = tokens[0] + " " + tokens[1] + " " + tokens[2];

        return fieldMap;
    }

    std::array<std::size_t, 3> parse_smectic_order_max_n(const std::map<std::string, std::string> &fieldMap) {
        if (!(fieldMap.find("max_n") != fieldMap.end()))
            return {5, 5, 5};

        auto maxNToneks = ParseUtils::tokenize<std::size_t>(fieldMap.at("max_n"));
        ValidateMsg(maxNToneks.size() == 1 || maxNToneks.size() == 3, "smectic order: max_n should be 1 or 3 ints");
        std::array<std::size_t, 3> maxN{};
        if (maxNToneks.size() == 1)
            maxN = {maxNToneks[0], maxNToneks[0], maxNToneks[0]};
        else // maxNToneks.size() == 3
            maxN = {maxNToneks[0], maxNToneks[1], maxNToneks[2]};

        bool anyNonzero = std::any_of(maxN.begin(), maxN.end(), [](int i) { return i != 0; });
        bool allNonNegative = std::all_of(maxN.begin(), maxN.end(), [](int i) { return i >= 0; });
        ValidateMsg(anyNonzero && allNonNegative, "All n ranges must be nonzero and some must be positive");

        return maxN;
    }

    std::unique_ptr<SmecticOrder> parse_smectic_order(std::istream &observableStream, const Version &version) {
        std::vector<std::string> tokens = ParseUtils::tokenize<std::string>(observableStream);
        auto fieldMap = ParseUtils::parseFields({"", "max_n", "dumpTauVector", "focalPoint"}, tokens);

        if (fieldMap.find("") != fieldMap.end())
            fieldMap = parse_smectic_order_old_syntax(tokens);

        std::array<std::size_t, 3> maxN = parse_smectic_order_max_n(fieldMap);

        bool dumpTauVector = false;
        if (fieldMap.find("dumpTauVector") != fieldMap.end()) {
            ValidateMsg(fieldMap["dumpTauVector"].empty(), SMECTIC_ORDER_USAGE);
            dumpTauVector = true;
        }

        std::string focalPoint = (version >= Version{0, 2, 0} ? "o" : "cm");
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
                throw InputError("Malformed Miller indices; format: nx.ny.nz");
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

    std::unique_ptr<BondOrder> parse_bond_order(std::istream &observableStream, const Version &version) {
        std::vector<std::string> tokens = ParseUtils::tokenize<std::string>(observableStream);
        auto fieldMap = ParseUtils::parseFields({"", "millerIdx", "ranks", "layeringPoint", "bondOrderPoint"}, tokens);

        if (fieldMap.find("") != fieldMap.end())
            fieldMap = parse_bond_order_old_syntax(tokens);

        ValidateMsg(fieldMap.find("millerIdx") != fieldMap.end() && fieldMap.find("ranks") != fieldMap.end(),
                    BOND_ORDER_USAGE);

        std::array<int, 3> millerIndices = parse_miller_indices(fieldMap.at("millerIdx"));
        std::vector<size_t> ranks = parse_bond_order_ranks(fieldMap);

        std::string layeringPoint = (version >= Version{0, 2, 0} ? "o" : "cm");
        if (fieldMap.find("layeringPoint") != fieldMap.end())
            layeringPoint = fieldMap["layeringPoint"];

        std::string bondOrderPoint = (version >= Version{0, 2, 0} ? "o" : "cm");
        if (fieldMap.find("bondOrderPoint") != fieldMap.end())
            bondOrderPoint = fieldMap["bondOrderPoint"];

        return std::make_unique<BondOrder>(ranks, millerIndices, layeringPoint, bondOrderPoint);
    }

    std::unique_ptr<PairEnumerator> parse_pair_enumerator(std::istream &observableStream, const Version &version) {
        std::string enumeratorName;
        observableStream >> enumeratorName;
        ValidateMsg(observableStream, BINNING_SPEC_USAGE);

        if (enumeratorName == "radial") {
            std::string focalPoint = (version >= Version{0, 2, 0} ? "o" : "cm");
            if (ParseUtils::isAnythingLeft(observableStream))
                observableStream >> focalPoint;
            ValidateMsg(observableStream, BINNING_SPEC_USAGE);

            return std::make_unique<RadialEnumerator>(focalPoint);
        } else if (enumeratorName == "layerwiseRadial") {
            std::string millerString;
            observableStream >> millerString;
            ValidateMsg(observableStream, BINNING_SPEC_USAGE);
            auto millerIndices = parse_miller_indices(millerString);

            std::string focalPoint = (version >= Version{0, 2, 0} ? "o" : "cm");
            if (ParseUtils::isAnythingLeft(observableStream))
                observableStream >> focalPoint;
            ValidateMsg(observableStream, BINNING_SPEC_USAGE);

            return std::make_unique<LayerwiseRadialEnumerator>(millerIndices, focalPoint);
        } else {
            throw InputError("Unknown binning specification: " + enumeratorName);
        }
    }

    std::unique_ptr<CorrelationFunction> parse_correlation_function(std::istream &observableStream) {
        std::string name;
        observableStream >> name;
        ValidateMsg(observableStream, CORR_FUN_USAGE);

        if (name == "S110") {
            std::string axisName;
            observableStream >> axisName;
            ValidateMsg(observableStream, CORR_FUN_USAGE);
            if (axisName == "primary")
                return std::make_unique<S110Correlation>(ShapeGeometry::Axis::PRIMARY);
            else if (axisName == "secondary")
                return std::make_unique<S110Correlation>(ShapeGeometry::Axis::SECONDARY);
            else if (axisName == "auxiliary")
                return std::make_unique<S110Correlation>(ShapeGeometry::Axis::AUXILIARY);
            else
                throw InputError(CORR_FUN_USAGE);
        } else {
            throw InputError("Unknown correlation function: " + name);
        }
    }

    std::unique_ptr<FourierTracker> parse_fourier_tracker(std::istream &observableStream) {
        std::array<std::size_t, 3> wavenumbers{};
        std::string functionName;
        observableStream >> wavenumbers[0] >> wavenumbers[1] >> wavenumbers[2] >> functionName;
        ValidateMsg(observableStream, FOURIER_TRACKER_USAGE);

        ValidateMsg(std::any_of(wavenumbers.begin(), wavenumbers.end(), [](std::size_t n) { return n > 0; }),
                    "Fourier tracker: at least one of wavenubmers must be > 0");

        FourierTracker::Function function;
        std::string functionShortName;
        if (functionName == "const") {
            function = [](const Shape &, const ShapeTraits &) -> double { return 1; };
            functionShortName = "c";
        } else if (functionName == "primaryAxis"
                   || functionName == "secondaryAxis"
                   || functionName == "auxiliaryAxis")
        {
            char coord{};
            observableStream >> coord;
            ValidateMsg(observableStream, FOURIER_TRACKER_USAGE);
            ValidateMsg(coord >= 'x' && coord <= 'z', "FourierTracker: axis coordinate should be x, y or z");
            std::size_t coordIdx = coord - 'x';

            if (functionName == "primaryAxis") {
                function = [coordIdx](const Shape &shape, const ShapeTraits &traits) {
                    return traits.getGeometry().getPrimaryAxis(shape)[coordIdx];
                };
                functionShortName = "pa_";
            } else if (functionName == "secondaryAxis") {
                function = [coordIdx](const Shape &shape, const ShapeTraits &traits) {
                    return traits.getGeometry().getSecondaryAxis(shape)[coordIdx];
                };
                functionShortName = "sa_";
            } else {   // auxiliaryAxis
                function = [coordIdx](const Shape &shape, const ShapeTraits &traits) {
                    return traits.getGeometry().getAuxiliaryAxis(shape)[coordIdx];
                };
                functionShortName = "aa_";
            }
            functionShortName += coord;
        } else {
            throw InputError(FOURIER_TRACKER_USAGE);
        }

        return std::make_unique<FourierTracker>(wavenumbers, function, functionShortName);
    }

    std::unique_ptr<GoldstoneTracker> parse_tracker(const std::string &trackerName, std::istream &trackerStream) {
        if (trackerName == "fourierTracker") {
            return parse_fourier_tracker(trackerStream);
        } else {
            throw InputError("Unknown Goldstone tracker: " + trackerName);
        }
    }

    std::unique_ptr<Observable> parse_observable(const std::string &observableName, std::istream &observableStream,
                                                 const Version &version)
    {
        if (observableName == "numberDensity") {
            return std::make_unique<NumberDensity>();
        } else if (observableName == "boxDimensions") {
            return std::make_unique<BoxDimensions>();
        } else if (observableName == "packingFraction") {
            return std::make_unique<PackingFraction>();
        } else if (observableName == "compressibilityFactor") {
            return std::make_unique<CompressibilityFactor>();
        } else if (observableName == "energyPerParticle") {
            return std::make_unique<EnergyPerParticle>();
        } else if (observableName == "energyFluctuationsPerParticle") {
            return std::make_unique<EnergyFluctuationsPerParticle>();
        } else if (observableName == "nematicOrder") {
            if (!ParseUtils::isAnythingLeft(observableStream))
                return std::make_unique<NematicOrder>();

            std::string QTensorString;
            observableStream >> QTensorString;
            ValidateMsg(QTensorString == "dumpQTensor", "Malformed nematic order, usage: nematicOrder (dumpQTensor)");
            return std::make_unique<NematicOrder>(true);
        } else if (observableName == "smecticOrder") {
            return parse_smectic_order(observableStream, version);
        } else if (observableName == "bondOrder") {
            return parse_bond_order(observableStream, version);
        } else if (observableName == "rotationMatrixDrift") {
            return std::make_unique<RotationMatrixDrift>();
        } else if (observableName == "temperature") {
            return std::make_unique<Temperature>();
        } else if (observableName == "pressure") {
            return std::make_unique<Pressure>();
        }

        try {
            return parse_tracker(observableName, observableStream);
        } catch (const InputError &) { }

        throw InputError("Unknown observable: " + observableName);
    }

    std::unique_ptr<DensityHistogram> parse_density_histogram(std::istream &observableStream,
                                                              std::size_t maxThreads)
    {
        auto fields = ParseUtils::parseFields({"n_bins", "tracker"}, observableStream);
        ValidateMsg(fields.find("n_bins") != fields.end(), DENSITY_HISTOGRAM_USAGE);
        auto nBinsTokens = ParseUtils::tokenize<std::size_t>(fields.at("n_bins"));
        ValidateMsg(nBinsTokens.size() == 3, DENSITY_HISTOGRAM_USAGE);
        std::array<std::size_t, 3> nBins{};
        std::copy(nBinsTokens.begin(), nBinsTokens.end(), nBins.begin());

        if (fields.find("tracker") == fields.end())
            return std::make_unique<DensityHistogram>(nBins, nullptr, maxThreads);

        std::istringstream trackerStream(fields.at("tracker"));
        std::string trackerName;
        trackerStream >> trackerName;
        ValidateMsg(trackerStream, DENSITY_HISTOGRAM_USAGE);
        std::unique_ptr<GoldstoneTracker> tracker = parse_tracker(trackerName, trackerStream);
        return std::make_unique<DensityHistogram>(nBins, std::move(tracker));
    }

    void parse_observables(const std::vector<std::string> &observables, ObservablesCollector &collector,
                           const Version &version)
    {
        std::regex typePattern(R"(^(?:inline|snapshot|averaging)(?:\/(?:inline|snapshot|averaging))*$)");

        for (auto observable : observables) {
            trim(observable);
            std::istringstream observableStream(observable);
            auto [observableName, observableType] = parse_observable_name_and_type(observableStream, typePattern);
            collector.addObservable(parse_observable(observableName, observableStream, version), observableType);
        }
    }

    void parse_bulk_observables(const std::vector<std::string> &bulkObservables, size_t maxThreads,
                                ObservablesCollector &collector, const Version &version)
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
                ValidateMsg(maxDistance > 0, "Max distance should be positive");
                ValidateMsg(numBins >= 2, "Number of bins should be >= 2");

                auto pairEnumerator = parse_pair_enumerator(observableStream, version);
                auto rhoCorr = std::make_unique<PairDensityCorrelation>(
                        std::move(pairEnumerator), maxDistance, numBins, maxThreads
                );
                collector.addBulkObservable(std::move(rhoCorr));
            } else if (observableName == "pairAveragedCorrelation") {
                double maxDistance{};
                std::size_t numBins{};
                observableStream >> maxDistance >> numBins;
                ValidateMsg(observableStream, PAIR_AVERAGED_CORRELATION_USAGE);
                ValidateMsg(maxDistance > 0, "Max distance should be positive");
                ValidateMsg(numBins >= 2, "Number of bins should be >= 2");

                auto correlationFunction = parse_correlation_function(observableStream);
                auto pairEnumerator = parse_pair_enumerator(observableStream, version);
                auto avgCorr = std::make_unique<PairAveragedCorrelation>(
                        std::move(pairEnumerator), std::move(correlationFunction), maxDistance, numBins, maxThreads
                );
                collector.addBulkObservable(std::move(avgCorr));
            } else if (observableName == "densityHistogram") {
                collector.addBulkObservable(parse_density_histogram(observableStream, maxThreads));
            } else {
                throw InputError("Unknown observable: " + observableName);
            }
        }
    }
}


namespace legacy {
    std::unique_ptr<ObservablesCollector>
    ObservablesCollectorFactory::create(const std::vector<std::string> &observables,
                                        const std::vector<std::string> &bulkObservables, std::size_t maxThreads,
                                        const Version &version)
    {
        auto collector = std::make_unique<ObservablesCollector>();
        parse_observables(observables, *collector, version);
        parse_bulk_observables(bulkObservables, maxThreads, *collector, version);
        return collector;
    }
}
