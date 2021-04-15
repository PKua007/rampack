//
// Created by Piotr Kubala on 12/12/2020.
//

#include <ostream>

#include "Shape.h"

void Shape::translate(const Vector<3> &translation, const BoundaryConditions &bc) {
    this->position += translation;
    this->position += bc.getCorrection(this->position);
}

void Shape::rotate(const Matrix<3, 3> &rotation) {
    this->orientation = rotation * this->orientation;
}

void Shape::applyBCTranslation(const BoundaryConditions &bc, Shape &other) const {
    other.position += bc.getTranslation(this->position, other.position);
}

void Shape::scale(double factor) {
    this->position *= factor;
}

void Shape::setPosition(const Vector<3> &position_) {
    this->position = position_;
}

void Shape::setOrientation(const Matrix<3, 3> &orientation_) {
    this->orientation = orientation_;
}

void Shape::scale(const std::array<double, 3> &factor) {
    this->position[0] *= factor[0];
    this->position[1] *= factor[1];
    this->position[2] *= factor[2];
}

std::ostream &operator<<(std::ostream &out, const Shape &shape) {
    out << "Shape{pos: " << shape.position << ", orientation: {";
    out << "{" << shape.orientation(0, 0) << ", " << shape.orientation(0, 1) << ", " << shape.orientation(0, 2) << "}, ";
    out << "{" << shape.orientation(1, 0) << ", " << shape.orientation(1, 1) << ", " << shape.orientation(1, 2) << "}, ";
    out << "{" << shape.orientation(2, 0) << ", " << shape.orientation(2, 1) << ", " << shape.orientation(2, 2) << "}";
    out << "}}";
    return out;
}

