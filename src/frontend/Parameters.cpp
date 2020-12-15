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
    // First we are looking for parameters from [general sections]
    auto config = Config::parse(input, '=', true);
    for (const auto &key : config.getKeys()) {
        if (key == "initialVolume")
            this->initialVolume = config.getDouble("initialVolume");
        else if (key == "numOfParticles")
            this->numOfParticles = config.getUnsignedLong("numOfParticles");
        else if (key == "temperature")
            this->temperature = config.getDouble("temperature");
        else if (key == "pressure")
            this->pressure = config.getDouble("pressure");
        else if (key == "positionStepSize")
            this->positionStepSize = config.getDouble("positionStepSize");
        else if (key == "volumeStepSize")
            this->volumeStepSize = config.getDouble("volumeStepSize");
        else if (key == "thermalisationSteps")
            this->thermalisationSteps = config.getUnsignedLong("thermalisationSteps");
        else if (key == "averagingSteps")
            this->averagingSteps = config.getUnsignedLong("averagingSteps");
        else if (key == "seed")
            this->seed = config.getUnsignedLong("seed");
        else if (key == "shapeName")
            this->shapeName = config.getString("shapeName");
        else if (key == "shapeAttributes")
            this->shapeAttributes = config.getString("shapeAttributes");
        else if (key == "wolframFilename")
            this->wolframFilename = config.getString("wolframFilename");
        else if (key == "compressibilityFilename")
            this->compressibilityFilename = config.getString("compressibilityFilename");
        else
            throw ParametersParseException("Unknown parameter " + key);
    }
    this->autocompleteAndValidate();
}

void Parameters::autocompleteAndValidate() {
    Validate(this->initialVolume > 0);
    Validate(this->numOfParticles > 0);
    Validate(this->temperature > 0);
    Validate(this->pressure > 0);
    Validate(this->positionStepSize > 0);
    Validate(this->volumeStepSize > 0);
    Validate(this->thermalisationSteps > 0);
    Validate(this->averagingSteps > 0);
}

void Parameters::print(Logger &logger) const {
    logger.info() << "initialVolume       : " << this->initialVolume << std::endl;
    logger.info() << "numOfParticles      : " << this->numOfParticles << std::endl;
    logger.info() << "temperature         : " << this->temperature << std::endl;
    logger.info() << "pressure            : " << this->pressure << std::endl;
    logger.info() << "positionStepSize    : " << this->pressure << std::endl;
    logger.info() << "volumeStepSize      : " << this->pressure << std::endl;
    logger.info() << "thermalisationSteps : " << this->thermalisationSteps << std::endl;
    logger.info() << "averagingSteps      : " << this->averagingSteps << std::endl;
    logger.info() << "seed                : " << this->seed << std::endl;
    logger.info() << "shapeName           : " << this->shapeName << std::endl;
    logger.info() << "shapeAttributes     : " << this->shapeAttributes << std::endl;
}