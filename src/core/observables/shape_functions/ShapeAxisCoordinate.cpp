//
// Created by Piotr Kubala on 23/03/2023.
//

#include "ShapeAxisCoordinate.h"
#include "utils/Exceptions.h"


ShapeAxisCoordinate::ShapeAxisCoordinate(ShapeGeometry::Axis axis, std::size_t coord) : axis{axis}, coord{coord} {
    Expects(coord <= 2);
    this->name = this->constructName();
}

void ShapeAxisCoordinate::calculate(const Shape &shape, const ShapeTraits &traits) {
    switch (this->axis) {
        case ShapeGeometry::Axis::PRIMARY:
            this->value = traits.getGeometry().getPrimaryAxis(shape)[this->coord];
            break;
        case ShapeGeometry::Axis::SECONDARY:
            this->value = traits.getGeometry().getSecondaryAxis(shape)[this->coord];
            break;
        case ShapeGeometry::Axis::AUXILIARY:
            this->value = traits.getGeometry().getAuxiliaryAxis(shape)[this->coord];
            break;
        default:
            AssertThrow("unreachable");
    }
}

std::string ShapeAxisCoordinate::constructName() const {
    std::string name_;
    using Axis = ShapeGeometry::Axis;
    switch (this->axis) {
        case Axis::PRIMARY:
            name_ = "pa_";
            break;
        case Axis::SECONDARY:
            name_ = "sa_";
            break;
        case Axis::AUXILIARY:
            name_ = "aa_";
            break;
    }

    auto compName = static_cast<char>('x' + this->coord);
    name_ += std::string{compName};
    return name_;
}
