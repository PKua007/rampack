//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_PARAMETERS_H
#define RAMPACK_PARAMETERS_H


#include <iosfwd>
#include <exception>
#include <map>
#include <string>
#include <variant>

#include "utils/Config.h"
#include "utils/Logger.h"


/**
 * @brief Exception thrown when parsing of accessing unknown parameter.
 */
struct ParametersParseException : public std::runtime_error {
public:
    explicit ParametersParseException(const std::string &what) : std::runtime_error(what) { }
};


/**
 * @brief Parameters which may be inherited from previous runs.
 */
class InheritableParameters {
protected:
    void validateInheritableParameters() const;
    void parseInheritableParameter(const std::string &key, const Config &config);

public:
    std::string moveTypes{};
    std::string scalingType{};
    double volumeStepSize{};
    std::string temperature{};
    std::string pressure{};

    void printInheritableParameters(Logger &logger) const;
};


/**
 * @brief A class which parses and stores parameters of the simulation.
 * @details The description of parameters can be found in the sample input.ini in the root folder
 */
class Parameters : public InheritableParameters {
private:
    void validate() const;
    [[nodiscard]] static std::pair<std::string, std::string> explodeRunSection(const std::string &runSectionName);

public:
    /**
     * @brief Parameters of the integration (Monte Carlo sampling) run.
     */
    class IntegrationParameters : public InheritableParameters {
    private:
        void validate() const;

    public:
        IntegrationParameters() = default;
        IntegrationParameters(const std::string &runName, const Config &runConfig);

        std::string runName{};
        std::size_t thermalisationCycles{};
        std::size_t averagingCycles{};
        std::size_t averagingEvery{};
        std::size_t snapshotEvery{};
        std::string observables{};
        std::string bulkObservables{};
        std::string packingFilename{};
        std::string wolframFilename{};
        std::string outputFilename{};
        std::string observableSnapshotFilename{};
        std::string bulkObservableFilenamePattern{};
        std::string recordingFilename{};

        void print(Logger &logger) const;
    };

    /**
     * @brief Parameters of overlap relaxation run.
     */
    class OverlapRelaxationParameters : public InheritableParameters {
    private:
        void validate() const;

    public:
        OverlapRelaxationParameters() = default;
        OverlapRelaxationParameters(const std::string &runName, const Config &runConfig);

        std::string runName{};
        std::size_t snapshotEvery{};
        std::string observables{};
        std::string bulkObservables{};
        std::string helperInteraction{};
        std::string packingFilename{};
        std::string wolframFilename{};
        std::string observableSnapshotFilename{};
        std::string bulkObservableFilenamePattern{};
        std::string recordingFilename{};

        void print(Logger &logger) const;
    };

    using RunParameters = std::variant<IntegrationParameters, OverlapRelaxationParameters>;

    /* All of these are described in input.ini */
    std::string initialDimensions{};
    std::string initialArrangement{};
    std::size_t numOfParticles{};
    std::string walls{};
    unsigned long seed{};
    std::string shapeName{};
    std::string shapeAttributes{};
    std::string interaction{};
    std::string scalingThreads = "1";
    std::string domainDivisions = "1 1 1";
    bool saveOnSignal = false;

    std::vector<RunParameters> runsParameters;

    Parameters() = default;

    /**
     * @brief Parses Config type input from @a input stream.
     */
    explicit Parameters(std::istream &input);

    /**
     * @brief Prints the summary of parameters to the @a out stream.
     */
    void print(Logger &logger) const;
};


#endif //RAMPACK_PARAMETERS_H
