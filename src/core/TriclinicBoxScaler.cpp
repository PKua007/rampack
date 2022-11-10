//
// Created by pkua on 09.11.22.
//

#include "TriclinicBoxScaler.h"
#include "utils/Assertions.h"


TriclinicBoxScaler::TriclinicBoxScaler(double stepSize) : stepSize{stepSize} {
    Expects(stepSize > 0);
}

bool TriclinicBoxScaler::increaseStepSize() {
    this->stepSize *= 1.1;
    return true;
}

bool TriclinicBoxScaler::decreaseStepSize() {
    this->stepSize /= 1.1;
    return true;
}

void TriclinicBoxScaler::setStepSize(double stepSize_) {
    Expects(stepSize_ > 0);
    this->stepSize = stepSize_;
}
