//
// Created by Michal Ciesla on 10/12/22.
//

#include <random>
#include "core/ShapeTraits.h"
#include "core/FreeBoundaryConditions.h"
#include "EVEstimator.h"

EVEstimator::EVEstimator(ShapeTraits &traits): range{traits.getInteraction().getRangeRadius()},
                                traits{traits},
                                interaction{const_cast<Interaction &>(traits.getInteraction())},
                                translationDistribution{std::uniform_real_distribution<double>(-traits.getInteraction().getRangeRadius(), traits.getInteraction().getRangeRadius())},
                                rotationDistribution{std::uniform_real_distribution<double>(0.0, 1.0)},
                                origin({0,0,0}, Matrix<3, 3>::identity()),
                                testShape({0,0,0}, Matrix<3, 3>::identity()) {
    this->intersectionCounter = 0;
    this->sampleCounter = 0;
    this->volume = 8*std::pow(this->range, 3.0);
}

void EVEstimator::calculateResults(){
    double p = ((double)this->intersectionCounter)/this->sampleCounter;
    this->expectedValue = p;
    this->variance = p*(1-p);
    this->result = p*this->volume;
    this->error = std::sqrt(this->variance/this->sampleCounter)*this->volume;
}


void EVEstimator::calculate(Matrix<3, 3, double> orientation, size_t samples){
    this->testShape.setOrientation(orientation);

    for(size_t i=0; i<samples; i++) {
        this->testShape.setPosition(Vector<3>{this->translationDistribution(this->mt), this->translationDistribution(this->mt), this->translationDistribution(this->mt)});
        this->intersectionCounter += this->interaction.overlapBetweenShapes(this->origin, this->testShape, this->fbc);
    }
    this->sampleCounter += samples;
    calculateResults();
}

void EVEstimator::calculate(size_t samples){
    for(size_t i=0; i<samples; i++) {
        this->testShape.setPosition(Vector<3>{this->translationDistribution(this->mt), this->translationDistribution(this->mt), this->translationDistribution(this->mt)});
        this->testShape.setOrientation(Matrix<3, 3>::rotation(
                2 * M_PI * this->rotationDistribution(this->mt),
                std::asin(2 * this->rotationDistribution(this->mt) - 1),
                2 * M_PI * this->rotationDistribution(this->mt)));
        this->intersectionCounter += this->interaction.overlapBetweenShapes(this->origin, this->testShape, this->fbc);
    }
    this->sampleCounter += samples;
    this->calculateResults();
}

void EVEstimator::calculate(Matrix<3, 3, double> orientation, double expectedError){
    this->testShape.setOrientation(orientation);

    size_t todoSamples = 1e3;
    do {
        this->calculate(orientation, todoSamples);
        this->calculateResults();
        // error = v*sigma/sqrt(n)
        size_t expectedSamples = std::pow(this->volume / expectedError, 2.0) * this->variance;
        if (expectedSamples < this->sampleCounter)
            todoSamples = 1e3;
        else
            todoSamples = expectedSamples - this->sampleCounter;
    }while(this->error > expectedError);
}

void EVEstimator::calculate(double expectedError){

    size_t todoSamples = 1e3;
    do {
        this->calculate(todoSamples);
        this->calculateResults();
        // error = v*sigma/sqrt(n)
        size_t expectedSamples = std::pow(this->volume / expectedError, 2.0) * this->variance;
        if (expectedSamples < this->sampleCounter)
            todoSamples = 1e3;
        else
            todoSamples = expectedSamples - this->sampleCounter;
    }while(this->error > expectedError);
}
double EVEstimator::getResult() const {
    return this->result;
}

double EVEstimator::getError() const {
    return this->error;
}
