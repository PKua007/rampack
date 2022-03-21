//
// Created by Piotr Kubala on 12/12/2020.
//

#include <string>
#include <ostream>

#include "Parameters.h"
#include "utils/Config.h"
#include "utils/Assertions.h"
#include "utils/Utils.h"

Parameters::Parameters(std::istream &input) {
    auto config = Config::parse(input, '=', true);

    auto generalConfig = config.fetchSubconfig("");
    for (const auto &key : generalConfig.getKeys()) {
        if (key == "initialDimensions")
            this->initialDimensions = generalConfig.getString("initialDimensions");
        else if (key == "numOfParticles")
            this->numOfParticles = generalConfig.getUnsignedLong("numOfParticles");
        else if (key == "initialArrangement")
            this->initialArrangement = generalConfig.getString("initialArrangement");
        else if (key == "moveTypes")
            this->moveTypes = generalConfig.getString("moveTypes");
        else if (key == "volumeStepSize")
            this->volumeStepSize = generalConfig.getDouble("volumeStepSize");
        else if (key == "seed")
            this->seed = generalConfig.getUnsignedLong("seed");
        else if (key == "shapeName")
            this->shapeName = generalConfig.getString("shapeName");
        else if (key == "shapeAttributes")
            this->shapeAttributes = generalConfig.getString("shapeAttributes");
        else if (key == "interaction")
            this->interaction = generalConfig.getString("interaction");
        else if (key == "scalingThreads")
            this->scalingThreads = generalConfig.getString("scalingThreads");
        else if (key == "domainDivisions")
            this->domainDivisions = generalConfig.getString("domainDivisions");
        else if (key == "scalingType")
            this->scalingType = generalConfig.getString("scalingType");
        else if (key == "saveOnSignal")
            this->saveOnSignal = generalConfig.getBoolean("saveOnSignal");
        else
            throw ParametersParseException("Unknown parameter " + key);
    }
    this->validate();

    auto runSections = config.getSections();
    for (const auto &runSection : runSections) {
        if (runSection.empty())
            continue;

        auto [runType, runName] = Parameters::explodeRunSection(runSection);
        auto runSubconfig = config.fetchSubconfig(runSection);

        if (runType == "integration" || runType == "run")
            this->runsParameters.emplace_back(IntegrationParameters(runName, runSubconfig));
        else if (runType == "overlaps")
            this->runsParameters.emplace_back(OverlapRelaxationParameters(runName, runSubconfig));
        else
            throw ParametersParseException("Unknown run type: \"" + runType + "\"");
    }
}

void Parameters::validate() const {
    Validate(this->numOfParticles > 0);
    Validate(this->volumeStepSize > 0);
    Validate(!this->scalingThreads.empty());
    Validate(!this->domainDivisions.empty());
    Validate(!this->scalingType.empty());
}

void Parameters::print(Logger &logger) const {
    logger.info() << "initialDimensions : " << this->initialDimensions << std::endl;
    logger.info() << "initialArangement : " << this->initialArrangement << std::endl;
    logger.info() << "numOfParticles    : " << this->numOfParticles << std::endl;
    logger.info() << "moveTypes         : " << this->moveTypes << std::endl;
    logger.info() << "volumeStepSize    : " << this->volumeStepSize << std::endl;
    logger.info() << "seed              : " << this->seed << std::endl;
    logger.info() << "shapeName         : " << this->shapeName << std::endl;
    logger.info() << "shapeAttributes   : " << this->shapeAttributes << std::endl;
    logger.info() << "interaction       : " << this->interaction << std::endl;
    logger.info() << "scalingThreads    : " << this->scalingThreads << std::endl;
    logger.info() << "domainDivisions   : " << this->domainDivisions << std::endl;
    logger.info() << "scalingType       : " << this->scalingType << std::endl;
    logger.info() << "saveOnSignal      : " << (this->saveOnSignal ? "true" : "false") << std::endl;
}

std::pair<std::string, std::string> Parameters::explodeRunSection(const std::string &runSectionName) {
    auto dotPos = runSectionName.find('.');
    if (dotPos == std::string::npos)
        throw ParametersParseException("Ini section [" + runSectionName + "] is not in format [runType.runName]");

    std::pair<std::string, std::string> result;
    result.first = runSectionName.substr(0, dotPos);
    result.second = runSectionName.substr(dotPos + 1);
    return result;
}

Parameters::IntegrationParameters::IntegrationParameters(const std::string &runName, const Config &runConfig) {
    this->runName = runName;
    for (const auto &key : runConfig.getKeys()) {
        if (key == "temperature")
            this->temperature = runConfig.getDouble("temperature");
        else if (key == "pressure")
            this->pressure = runConfig.getDouble("pressure");
        else if (key == "thermalisationCycles")
            this->thermalisationCycles = runConfig.getUnsignedLong("thermalisationCycles");
        else if (key == "averagingCycles")
            this->averagingCycles = runConfig.getUnsignedLong("averagingCycles");
        else if (key == "averagingEvery")
            this->averagingEvery = runConfig.getUnsignedLong("averagingEvery");
        else if (key == "snapshotEvery")
            this->snapshotEvery = runConfig.getUnsignedLong("snapshotEvery");
        else if (key == "observables")
            this->observables = runConfig.getString("observables");
        else if (key == "wolframFilename")
            this->wolframFilename = runConfig.getString("wolframFilename");
        else if (key == "packingFilename")
            this->packingFilename = runConfig.getString("packingFilename");
        else if (key == "outputFilename")
            this->outputFilename = runConfig.getString("outputFilename");
        else if (key == "observableSnapshotFilename")
            this->observableSnapshotFilename = runConfig.getString("observableSnapshotFilename");
        else
            throw ParametersParseException("Unknown parameter " + key);
    }
    this->validate();
}

void Parameters::IntegrationParameters::validate() const {
    Validate(this->temperature > 0);
    Validate(this->pressure > 0);
    Validate(this->thermalisationCycles > 0);
    Validate(this->averagingCycles > 0);
    Validate(this->averagingEvery > 0);
    Validate(this->snapshotEvery > 0);
}

void Parameters::IntegrationParameters::print(Logger &logger) const {
    logger.info() << "temperature             : " << this->temperature << std::endl;
    logger.info() << "pressure                : " << this->pressure << std::endl;
    logger.info() << "thermalisationCycles    : " << this->thermalisationCycles << std::endl;
    logger.info() << "averagingCycles         : " << this->averagingCycles << std::endl;
    logger.info() << "averagingEvery          : " << this->averagingEvery << std::endl;
    logger.info() << "snapshotEvery           : " << this->snapshotEvery << std::endl;
    logger.info() << "observables             : " << this->observables << std::endl;
    logger.info() << "packingFilename         : " << this->packingFilename << std::endl;
    logger.info() << "wolframFilename         : " << this->wolframFilename << std::endl;
    logger.info() << "outputFilename          : " << this->outputFilename << std::endl;
    logger.info() << "observableSnapshotFilename : " << this->observableSnapshotFilename << std::endl;
}

Parameters::OverlapRelaxationParameters::OverlapRelaxationParameters(const std::string &runName, const Config &runConfig) {
    this->runName = runName;
    for (const auto &key : runConfig.getKeys()) {
        if (key == "temperature")
            this->temperature = runConfig.getDouble("temperature");
        else if (key == "pressure")
            this->pressure = runConfig.getDouble("pressure");
        else if (key == "snapshotEvery")
            this->snapshotEvery = runConfig.getUnsignedLong("snapshotEvery");
        else if (key == "observables")
            this->observables = runConfig.getString("observables");
        else if (key == "helperInteraction")
            this->helperInteraction = runConfig.getString("helperInteraction");
        else if (key == "wolframFilename")
            this->wolframFilename = runConfig.getString("wolframFilename");
        else if (key == "packingFilename")
            this->packingFilename = runConfig.getString("packingFilename");
        else if (key == "observableSnapshotFilename")
            this->observableSnapshotFilename = runConfig.getString("observableSnapshotFilename");
        else
            throw ParametersParseException("Unknown parameter " + key);
    }
    this->validate();
}

void Parameters::OverlapRelaxationParameters::validate() const {
    Validate(this->temperature > 0);
    Validate(this->pressure > 0);
    Validate(this->snapshotEvery > 0);
}

void Parameters::OverlapRelaxationParameters::print(Logger &logger) const {
    logger.info() << "temperature             : " << this->temperature << std::endl;
    logger.info() << "pressure                : " << this->pressure << std::endl;
    logger.info() << "snapshotEvery           : " << this->snapshotEvery << std::endl;
    logger.info() << "observables             : " << this->observables << std::endl;
    logger.info() << "helperInteraction       : " << this->helperInteraction << std::endl;
    logger.info() << "packingFilename         : " << this->packingFilename << std::endl;
    logger.info() << "wolframFilename         : " << this->wolframFilename << std::endl;
    logger.info() << "observableSnapshotFilename : " << this->observableSnapshotFilename << std::endl;
}