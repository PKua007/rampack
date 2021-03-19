//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_PARAMETERS_H
#define RAMPACK_PARAMETERS_H


#include <iosfwd>
#include <exception>
#include <map>
#include <string>

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
 * @brief A class which parses and stores parameters of the simulation.
 * @details The description of parameters can be found in the sample input.ini in the root folder
 */
class Parameters {
private:
    void validate() const;

public:
    class RunParameters {
    private:
        void validate() const;

    public:
        RunParameters() = default;
        RunParameters(const std::string &runName, const Config &runConfig);

        std::string runName{};
        double temperature{};
        double pressure{};
        std::size_t thermalisationCycles{};
        std::size_t averagingCycles{};
        std::size_t averagingEvery{};
        std::string packingFilename{};
        std::string wolframFilename{};
        std::string outputFilename{};
        std::string densitySnapshotFilename{};

        void print(Logger &logger) const;
    };

    /* All of these are described in input.ini */
    std::string initialDimensions{};
    std::string initialArrangement{};
    std::size_t numOfParticles{};
    double positionStepSize{};
    double rotationStepSize{};
    double volumeStepSize{};
    unsigned long seed{};
    std::string shapeName{};
    std::string shapeAttributes{};
    std::string interaction{};
    std::string scalingThreads = "1";
    std::string domainDivisions = "1 1 1";
    std::string scalingType = "isotropic";

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
