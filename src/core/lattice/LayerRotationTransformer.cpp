//
// Created by pkua on 19.05.22.
//

#include <numeric>

#include "LayerRotationTransformer.h"
#include "LatticeTraits.h"
#include "utils/Exceptions.h"


LayerRotationTransformer::LayerRotationTransformer(const LatticeTraits::Axis layerAxis,
                                                   const LatticeTraits::Axis rotationAxis,
                                                   const RotationAngle& rotationAngle, const bool isAlternating,
                                                   const bool isCumulative)
        : LayerWiseTransformer(layerAxis), rotationAxisIdx{LatticeTraits::axisToIndex(rotationAxis)},
          rotationAngle{rotationAngle}, isAlternating{isAlternating}, isCumulative{isCumulative}
{ }

std::optional<std::size_t> LayerRotationTransformer::getRequestedNumOfLayers() const {
    if (this->isAlternating)
        return 2;
    if (!this->isCumulative)
        return 1;

    if (std::holds_alternative<Radians>(this->rotationAngle))
        return std::nullopt;
    if (const auto *quotient = std::get_if<FullRotationQuotient>(&this->rotationAngle))
        return quotient->denominator;
    AssertThrow("valueless by exception");
}

LayerRotationTransformer::FullRotationQuotient::FullRotationQuotient(const int numerator, const unsigned denominator)
        : numerator{numerator}, denominator{denominator}
{
    Expects(denominator != 0);

    const unsigned gcd = std::gcd(this->numerator, this->denominator);
    const_cast<int&>(this->numerator) /= gcd;
    const_cast<unsigned&>(this->denominator) /= gcd;
}

double LayerRotationTransformer::getRotationAngle() const {
    if (const auto *radianAngle = std::get_if<Radians>(&this->rotationAngle))
        return *radianAngle;
    if (const auto *quotientAngle = std::get_if<FullRotationQuotient>(&this->rotationAngle))
        return quotientAngle->toRadians();
    AssertThrow("valueless by exception");
}

Shape LayerRotationTransformer::transformShape(const Shape &shape, const std::size_t layerIdx) const {
    if (!this->isAlternating && !this->isCumulative)
        Assert(layerIdx == 0);

    const double radRotationAngle = this->getRotationAngle();
    std::array<double, 3> angles{};
    angles.fill(0);

    if (this->isAlternating) {
        switch (layerIdx) {
            case 0:
                angles[this->rotationAxisIdx] = (this->isCumulative ? 0 : radRotationAngle);
                break;
            case 1:
                angles[this->rotationAxisIdx] = -radRotationAngle;
                break;
            default:
                AssertThrow("layerIdx = " + std::to_string(layerIdx));
        }
    } else if (this->isCumulative) {
        angles[this->rotationAxisIdx] = radRotationAngle * static_cast<double>(layerIdx);
    } else {
        angles[this->rotationAxisIdx] = radRotationAngle;
    }

    auto newShape = shape;
    newShape.rotate(Matrix<3, 3>::rotation(angles[0], angles[1], angles[2]));
    return newShape;
}
