//
// Created by Michal Ciesla on 10/12/22.
//

#ifndef RAMPACK_EVESTIMATOR_H
#define RAMPACK_EVESTIMATOR_H

#include <random>
#include <omp.h>
#include <queue>
#include "core/Shape.h"
#include "core/ShapeTraits.h"
#include "core/FreeBoundaryConditions.h"
#include "EVBox.h"

class EVEstimator {
private:
    ShapeTraits &traits;
    const Interaction &interaction;
    std::uniform_real_distribution<double> translationDistribution;
    std::uniform_real_distribution<double> u01Distribution;
    std::mt19937 mt;
    Shape origin, testShape;
    FreeBoundaryConditions fbc;

    size_t intersectionCounter, sampleCounter;

    double result, error;

    void calculateMCResults(double volume);
    void calculateBoxResults(std::queue<const EVBox *> *partiallyCoveredBoxes, double coveredVolume);

public:
    explicit EVEstimator(ShapeTraits &traits);
    void clear();
    void calculateMC(Matrix<3, 3, double> *orientation, size_t samples);
    void calculateMC(Matrix<3, 3, double> *orientation, double expectedError);
    void calculateBox(Matrix<3, 3, double> *orientation, double expectedError);

    [[nodiscard]] double getResult() const;
    [[nodiscard]] double getError() const;
};
#endif //RAMPACK_EVESTIMATOR_H
