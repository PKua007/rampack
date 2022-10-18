//
// Created by Michal Ciesla on 10/12/22.
//

#ifndef RAMPACK_EVESTIMATOR_H
#define RAMPACK_EVESTIMATOR_H

#include <random>
#include "core/Shape.h"
#include "core/ShapeTraits.h"
#include "core/FreeBoundaryConditions.h"

class EVEstimator {
private:
    double range, volume;
    ShapeTraits &traits;
    const Interaction &interaction;
    std::uniform_real_distribution<double> translationDistribution;
    std::uniform_real_distribution<double> rotationDistribution;
    std::mt19937 mt;
    Shape origin, testShape;
    FreeBoundaryConditions fbc;

    size_t intersectionCounter, sampleCounter;

    double result, error, expectedValue, variance;

    void calculateResults();

public:
    EVEstimator(ShapeTraits &traits);
    void calculate(Matrix<3, 3, double> orientation, size_t samples);
    void calculate(Matrix<3, 3, double> orientation, double expectedError);

    void calculate(size_t samples);
    void calculate(double expectedError);

    [[nodiscard]] double getResult() const;
    [[nodiscard]] double getError() const;
};
#endif //RAMPACK_EVESTIMATOR_H
