//
// Created by pkua on 19.05.22.
//

#include "LayerRotationTransformer.h"
#include "LatticeTraits.h"


LayerRotationTransformer::LayerRotationTransformer(LatticeTraits::Axis layerAxis, LatticeTraits::Axis rotationAxis,
                                                   double rotationAngle, bool isAlternating)
        : LayerWiseTransformer(layerAxis), rotationAxisIdx{LatticeTraits::axisToIndex(rotationAxis)},
          rotationAngle{rotationAngle}, isAlternating{isAlternating}
{ }

std::size_t LayerRotationTransformer::getRequestedNumOfLayers() const {
    return this->isAlternating ? 2 : 1;
}

Shape LayerRotationTransformer::transformShape(const Shape &shape, std::size_t layerIdx) const {
    if (!this->isAlternating)
        Assert(layerIdx == 0);

    std::array<double, 3> angles{};
    angles.fill(0);
    switch (layerIdx) {
        case 0:
            angles[this->rotationAxisIdx] = this->rotationAngle;
            break;
        case 1:
            angles[this->rotationAxisIdx] = -this->rotationAngle;
            break;
        default:
            AssertThrow("layerIdx = " + std::to_string(layerIdx));
    }

    auto newShape = shape;
    newShape.rotate(Matrix<3, 3>::rotation(angles[0], angles[1], angles[2]));
    return newShape;
}
