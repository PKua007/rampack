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
        else if (key == "rotationStepSize")
            this->rotationStepSize = config.getDouble("rotationStepSize");
        else if (key == "volumeStepSize")
            this->volumeStepSize = config.getDouble("volumeStepSize");
        else if (key == "thermalisationCycles")
            this->thermalisationCycles = config.getUnsignedLong("thermalisationCycles");
        else if (key == "averagingCycles")
            this->averagingCycles = config.getUnsignedLong("averagingCycles");
        else if (key == "averagingEvery")
            this->averagingEvery = config.getUnsignedLong("averagingEvery");
        else if (key == "seed")
            this->seed = config.getUnsignedLong("seed");
        else if (key == "shapeName")
            this->shapeName = config.getString("shapeName");
        else if (key == "shapeAttributes")
            this->shapeAttributes = config.getString("shapeAttributes");
        else if (key == "interaction")
            this->interaction = config.getString("interaction");
        else if (key == "wolframFilename")
            this->wolframFilename = config.getString("wolframFilename");
        else if (key == "compressibilityFilename")
            this->compressibilityFilename = config.getString("compressibilityFilename");
        else if (key == "densitySnapshotFilename")
            this->densitySnapshotFilename = config.getString("densitySnapshotFilename");
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
    Validate(this->rotationStepSize > 0);
    Validate(this->volumeStepSize > 0);
    Validate(this->thermalisationCycles > 0);
    Validate(this->averagingCycles > 0);
    Validate(this->averagingEvery > 0);
}

void Parameters::print(Logger &logger) const {
    logger.info() << "initialVolume        : " << this->initialVolume << std::endl;
    logger.info() << "numOfParticles       : " << this->numOfParticles << std::endl;
    logger.info() << "temperature          : " << this->temperature << std::endl;
    logger.info() << "pressure             : " << this->pressure << std::endl;
    logger.info() << "positionStepSize     : " << this->positionStepSize << std::endl;
    logger.info() << "rotationStepSize     : " << this->rotationStepSize << std::endl;
    logger.info() << "volumeStepSize       : " << this->volumeStepSize << std::endl;
    logger.info() << "thermalisationCycles : " << this->thermalisationCycles << std::endl;
    logger.info() << "averagingCycles      : " << this->averagingCycles << std::endl;
    logger.info() << "averagingEvery       : " << this->averagingEvery << std::endl;
    logger.info() << "seed                 : " << this->seed << std::endl;
    logger.info() << "shapeName            : " << this->shapeName << std::endl;
    logger.info() << "shapeAttributes      : " << this->shapeAttributes << std::endl;
    logger.info() << "interaction          : " << this->interaction << std::endl;
}