//
// Created by Piotr Kubala on 23/03/2023.
//

#include "ShapeAxisCoordinate.h"
#include "utils/Exceptions.h"


ShapeAxisCoordinate::ShapeAxisCoordinate(ShapeGeometry::Axis axis, std::size_t coord) : axis{axis}, coord{coord} {
    Expects(coord <= 2);
}

double ShapeAxisCoordinate::calculate(const Shape &shape, const ShapeTraits &traits) const {
    switch (this->axis) {
        case ShapeGeometry::Axis::PRIMARY:
            return traits.getGeometry().getPrimaryAxis(shape)[this->coord];
        case ShapeGeometry::Axis::SECONDARY:
            return traits.getGeometry().getSecondaryAxis(shape)[this->coord];
        case ShapeGeometry::Axis::AUXILIARY:
            return traits.getGeometry().getAuxiliaryAxis(shape)[this->coord];
    }
    AssertThrow("");
}
