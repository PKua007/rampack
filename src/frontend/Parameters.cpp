//
// Created by Piotr Kubala on 12/12/2020.
//

#include "Parameters.h"

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
        if (key == "boxSize")
            this->boxSize = config.getDouble("boxSize");
        else
            throw ParametersParseException("Unknown parameter " + key);
    }
    this->autocompleteAndValidate();
}

void Parameters::autocompleteAndValidate() {
    Validate(this->boxSize > 0);
}

void Parameters::print(Logger &logger) const {
    logger.info() << "boxSize : " << this->boxSize << std::endl;
}