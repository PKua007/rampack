//
// Created by pkua on 09.04.2022.
//

#include <numeric>

#include "RotationMatrixDrift.h"


void RotationMatrixDrift::calculate(const Packing &packing, [[maybe_unused]] double temperature,
                                    [[maybe_unused]] double pressure, [[maybe_unused]] const ShapeTraits &shapeTraits)
{
    this->frobenius2min = std::numeric_limits<double>::infinity();
    this->frobenius2max = 0;

    auto frobeniusAccumulator = [this](double sum, const Shape &shape) {
        const auto &rot = shape.getOrientation();
        auto rotError = rot*rot.transpose() - Matrix<3, 3>::identity();
        double rotErrorNorm2 = rotError.norm2();

        if (rotErrorNorm2 < this->frobenius2min)
            this->frobenius2min = rotErrorNorm2;
        if (rotErrorNorm2 > this->frobenius2max)
            this->frobenius2max = rotErrorNorm2;

        return sum + rotErrorNorm2;
    };

    this->frobenius2 = std::accumulate(packing.begin(), packing.end(), 0.0, frobeniusAccumulator);
    this->frobenius2 /= static_cast<double>(packing.size());
}
