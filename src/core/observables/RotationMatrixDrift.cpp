//
// Created by pkua on 09.04.2022.
//

#include <numeric>

#include "RotationMatrixDrift.h"


void RotationMatrixDrift::calculate(const Packing &packing, [[maybe_unused]] double temperature,
                                    [[maybe_unused]] double pressure, [[maybe_unused]] const ShapeTraits &shapeTraits)
{
    auto frobeniusAccumulator = [](double sum, const Shape &shape) {
        const auto &rot = shape.getOrientation();
        auto rotError = rot*rot.transpose() - Matrix<3, 3>::identity();
        return sum + rotError.norm2();
    };
    this->frobenius2 = std::accumulate(packing.begin(), packing.end(), 0.0, frobeniusAccumulator);
    this->frobenius2 /= static_cast<double>(packing.size());
}
