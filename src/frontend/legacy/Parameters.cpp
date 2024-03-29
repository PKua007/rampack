//
// Created by Piotr Kubala on 12/12/2020.
//

#include <string>
#include <ostream>

#include "Parameters.h"
#include "utils/Config.h"
#include "utils/Exceptions.h"
#include "utils/Utils.h"


void InheritableParameters::parseInheritableParameter(const std::string &key, const Config &config) {
    if (key == "moveTypes")
        this->moveTypes = config.getString("moveTypes");
    else if (key == "scalingType")
        this->scalingType = config.getString("scalingType");
    else if (key == "volumeStepSize")
        this->volumeStepSize = config.getDouble("volumeStepSize");
    else if (key == "temperature")
        this->temperature = config.getString("temperature");
    else if (key == "pressure")
        this->pressure = config.getString("pressure");
    else
        throw ParametersParseException("Unknown parameter " + key);
}

void InheritableParameters::validateInheritableParameters() const {

}

void InheritableParameters::printInheritableParameters(Logger &logger) const {
    logger.info() << "-- Inheritable parameters:" << std::endl;
    logger.info() << "moveTypes      : " << moveTypes << std::endl;
    logger.info() << "scalingType    : " << scalingType << std::endl;
    logger.info() << "volumeStepSize : " << volumeStepSize << std::endl;
    logger.info() << "temperature    : " << temperature << std::endl;
    logger.info() << "pressure       : " << pressure << std::endl;
}

Parameters::Parameters(std::istream &input) {
    auto config = Config::parse(input, '=', true);

    auto generalConfig = config.fetchSubconfig("");
    for (const auto &key : generalConfig.getKeys()) {
        if (key == "version")
            this->version = generalConfig.getString("version");
        else if (key == "initialDimensions")
            this->initialDimensions = generalConfig.getString("initialDimensions");
        else if (key == "numOfParticles")
            this->numOfParticles = generalConfig.getUnsignedLong("numOfParticles");
        else if (key == "initialArrangement")
            this->initialArrangement = generalConfig.getString("initialArrangement");
        else if (key == "walls")
            this->walls = generalConfig.getString("walls");
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
            this->parseInheritableParameter(key, config);
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
    if (this->version < Version{0, 8, 0})
        ValidateMsg(this->numOfParticles > 0, "'numOfParticles' should be positive");
    ValidateMsg(!this->scalingThreads.empty(), "'scalingThread' should be non-empty");
    ValidateMsg(!this->domainDivisions.empty(), "'domainDivisions' should be non-empty");
    this->validateInheritableParameters();
}

void Parameters::print(Logger &logger) const {
    this->printInheritableParameters(logger);
    logger.info() << "-- Main parameters:" << std::endl;
    logger.info() << "version            : " << this->version << std::endl;
    logger.info() << "initialDimensions  : " << this->initialDimensions << std::endl;
    logger.info() << "initialArrangement : " << this->initialArrangement << std::endl;
    logger.info() << "numOfParticles     : " << this->numOfParticles << std::endl;
    logger.info() << "walls              : " << this->walls << std::endl;
    logger.info() << "seed               : " << this->seed << std::endl;
    logger.info() << "shapeName          : " << this->shapeName << std::endl;
    logger.info() << "shapeAttributes    : " << this->shapeAttributes << std::endl;
    logger.info() << "interaction        : " << this->interaction << std::endl;
    logger.info() << "scalingThreads     : " << this->scalingThreads << std::endl;
    logger.info() << "domainDivisions    : " << this->domainDivisions << std::endl;
    logger.info() << "saveOnSignal       : " << (this->saveOnSignal ? "true" : "false") << std::endl;
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
        if (key == "thermalisationCycles")
            this->thermalisationCycles = runConfig.getUnsignedLong("thermalisationCycles");
        else if (key == "averagingCycles")
            this->averagingCycles = runConfig.getUnsignedLong("averagingCycles");
        else if (key == "averagingEvery")
            this->averagingEvery = runConfig.getUnsignedLong("averagingEvery");
        else if (key == "snapshotEvery")
            this->snapshotEvery = runConfig.getUnsignedLong("snapshotEvery");
        else if (key == "inlineInfoEvery")
            this->inlineInfoEvery = runConfig.getUnsignedLong("inlineInfoEvery");
        else if (key == "orientationFixEvery")
            this->orientationFixEvery = runConfig.getUnsignedLong("orientationFixEvery");
        else if (key == "observables")
            this->observables = runConfig.getString("observables");
        else if (key == "bulkObservables")
            this->bulkObservables = runConfig.getString("bulkObservables");
        else if (key == "wolframFilename")
            this->wolframFilename = runConfig.getString("wolframFilename");
        else if (key == "packingFilename")
            this->packingFilename = runConfig.getString("packingFilename");
        else if (key == "xyzPackingFilename")
            this->xyzPackingFilename = runConfig.getString("xyzPackingFilename");
        else if (key == "outputFilename")
            this->outputFilename = runConfig.getString("outputFilename");
        else if (key == "observableSnapshotFilename")
            this->observableSnapshotFilename = runConfig.getString("observableSnapshotFilename");
        else if (key == "bulkObservableFilenamePattern")
            this->bulkObservableFilenamePattern = runConfig.getString("bulkObservableFilenamePattern");
        else if (key == "recordingFilename")
            this->recordingFilename = runConfig.getString("recordingFilename");
        else if (key == "xyzRecordingFilename")
            this->xyzRecordingFilename = runConfig.getString("xyzRecordingFilename");
        else
            this->parseInheritableParameter(key, runConfig);
    }
    this->validate();
}

void Parameters::IntegrationParameters::validate() const {
    ValidateMsg(this->thermalisationCycles > 0 || this->averagingCycles > 0,
                "At least one of: 'thermalisationCycles', 'averagingCycles' should be positive");
    ValidateMsg(this->averagingEvery > 0, "'averagingEvery' should be positive");
    ValidateMsg(this->snapshotEvery > 0, "'snapshotEvery' should be positive");
    ValidateMsg(this->inlineInfoEvery > 0, "'inlineInfoEvery' should be positive");
    ValidateMsg(this->orientationFixEvery > 0, "'orientationFixEvery' should be positive");
    this->validateInheritableParameters();
}

void Parameters::IntegrationParameters::print(Logger &logger) const {
    this->printInheritableParameters(logger);
    logger.info() << "-- Integration parameters:" << std::endl;
    logger.info() << "thermalisationCycles          : " << this->thermalisationCycles << std::endl;
    logger.info() << "averagingCycles               : " << this->averagingCycles << std::endl;
    logger.info() << "averagingEvery                : " << this->averagingEvery << std::endl;
    logger.info() << "snapshotEvery                 : " << this->snapshotEvery << std::endl;
    logger.info() << "inlineInfoEvery               : " << this->inlineInfoEvery << std::endl;
    logger.info() << "orientationFixEvery           : " << this->orientationFixEvery << std::endl;
    logger.info() << "observables                   : " << this->observables << std::endl;
    logger.info() << "bulkObservables               : " << this->bulkObservables << std::endl;
    logger.info() << "packingFilename               : " << this->packingFilename << std::endl;
    logger.info() << "xyzPackingFilename            : " << this->xyzPackingFilename << std::endl;
    logger.info() << "wolframFilename               : " << this->wolframFilename << std::endl;
    logger.info() << "outputFilename                : " << this->outputFilename << std::endl;
    logger.info() << "observableSnapshotFilename    : " << this->observableSnapshotFilename << std::endl;
    logger.info() << "bulkObservableFilenamePattern : " << this->bulkObservableFilenamePattern << std::endl;
    logger.info() << "recordingFilename             : " << this->recordingFilename << std::endl;
    logger.info() << "xyzRecordingFilename          : " << this->xyzRecordingFilename << std::endl;
}

Parameters::OverlapRelaxationParameters::OverlapRelaxationParameters(const std::string &runName, const Config &runConfig) {
    this->runName = runName;
    for (const auto &key : runConfig.getKeys()) {
        if (key == "snapshotEvery")
            this->snapshotEvery = runConfig.getUnsignedLong("snapshotEvery");
        else if (key == "inlineInfoEvery")
            this->inlineInfoEvery = runConfig.getUnsignedLong("inlineInfoEvery");
        else if (key == "orientationFixEvery")
            this->orientationFixEvery = runConfig.getUnsignedLong("orientationFixEvery");
        else if (key == "observables")
            this->observables = runConfig.getString("observables");
        else if (key == "bulkObservables")
            this->bulkObservables = runConfig.getString("bulkObservables");
        else if (key == "helperInteraction")
            this->helperInteraction = runConfig.getString("helperInteraction");
        else if (key == "wolframFilename")
            this->wolframFilename = runConfig.getString("wolframFilename");
        else if (key == "packingFilename")
            this->packingFilename = runConfig.getString("packingFilename");
        else if (key == "xyzPackingFilename")
            this->xyzPackingFilename = runConfig.getString("xyzPackingFilename");
        else if (key == "observableSnapshotFilename")
            this->observableSnapshotFilename = runConfig.getString("observableSnapshotFilename");
        else if (key == "recordingFilename")
            this->recordingFilename = runConfig.getString("recordingFilename");
        else if (key == "xyzRecordingFilename")
            this->xyzRecordingFilename = runConfig.getString("xyzRecordingFilename");
        else
            this->parseInheritableParameter(key, runConfig);
    }
    this->validate();
}

void Parameters::OverlapRelaxationParameters::validate() const {
    ValidateMsg(this->snapshotEvery > 0, "'snapshotEvery' should be positive");
    ValidateMsg(this->inlineInfoEvery > 0, "'inlineInfoEvery' should be positive");
    ValidateMsg(this->orientationFixEvery > 0, "'orientationFixEvery' should be positive");
    this->validateInheritableParameters();
}

void Parameters::OverlapRelaxationParameters::print(Logger &logger) const {
    this->printInheritableParameters(logger);
    logger.info() << "-- Overlap relaxation parameters:" << std::endl;
    logger.info() << "snapshotEvery                 : " << this->snapshotEvery << std::endl;
    logger.info() << "inlineInfoEvery               : " << this->inlineInfoEvery << std::endl;
    logger.info() << "orientationFixEvery           : " << this->orientationFixEvery << std::endl;
    logger.info() << "observables                   : " << this->observables << std::endl;
    logger.info() << "bulkObservables               : " << this->bulkObservables << std::endl;
    logger.info() << "helperInteraction             : " << this->helperInteraction << std::endl;
    logger.info() << "packingFilename               : " << this->packingFilename << std::endl;
    logger.info() << "xyzPackingFilename            : " << this->xyzPackingFilename << std::endl;
    logger.info() << "wolframFilename               : " << this->wolframFilename << std::endl;
    logger.info() << "observableSnapshotFilename    : " << this->observableSnapshotFilename << std::endl;
    logger.info() << "recordingFilename             : " << this->recordingFilename << std::endl;
    logger.info() << "xyzRecordingFilename          : " << this->xyzRecordingFilename << std::endl;
}