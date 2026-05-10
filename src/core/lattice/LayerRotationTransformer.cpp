//
// Created by pkua on 19.05.22.
//

#include <numeric>

#include "LayerRotationTransformer.h"
#include "LatticeTraits.h"
#include "utils/Exceptions.h"


LayerRotationTransformer::LayerRotationTransformer(LatticeTraits::Axis layerAxis, LatticeTraits::Axis rotationAxis,
                                                   const RotationAngle& rotationAngle, bool isAlternating)
        : LayerWiseTransformer(layerAxis), rotationAxisIdx{LatticeTraits::axisToIndex(rotationAxis)},
          rotationAngle{rotationAngle}, isAlternating{isAlternating}
{ }

std::size_t LayerRotationTransformer::getRequestedNumOfLayers() const {
    return this->isAlternating ? 2 : 1;
}

LayerRotationTransformer::FullRotationQuotient::FullRotationQuotient(const unsigned numerator, const unsigned denominator)
        : numerator{numerator}, denominator{denominator}
{
    Expects(denominator != 0);

    const unsigned gcd = std::gcd(this->numerator, this->denominator);
    const_cast<unsigned&>(this->numerator) /= gcd;
    const_cast<unsigned&>(this->denominator) /= gcd;
}

double LayerRotationTransformer::getRotationAngle() const {
    if (const auto *radianAngle = std::get_if<Radians>(&this->rotationAngle))
        return *radianAngle;
    if (const auto *quotientAngle = std::get_if<FullRotationQuotient>(&this->rotationAngle))
        return quotientAngle->toRadians();
    AssertThrow("valueless by exception");
}

Shape LayerRotationTransformer::transformShape(const Shape &shape, std::size_t layerIdx) const {
    if (!this->isAlternating)
        Assert(layerIdx == 0);

    const double radRotationAngle = this->getRotationAngle();
    std::array<double, 3> angles{};
    angles.fill(0);
    switch (layerIdx) {
        case 0:
            angles[this->rotationAxisIdx] = radRotationAngle;
            break;
        case 1:
            angles[this->rotationAxisIdx] = -radRotationAngle;
            break;
        default:
            AssertThrow("layerIdx = " + std::to_string(layerIdx));
    }

    auto newShape = shape;
    newShape.rotate(Matrix<3, 3>::rotation(angles[0], angles[1], angles[2]));
    return newShape;
}
