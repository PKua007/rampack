//
// Created by Piotr Kubala on 12/12/2020.
//

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

