//
// Created by Piotr Kubala on 12/12/2020.
//

#include <algorithm>
#include <numeric>

#include "Shape.h"

void Shape::translate(const Vector<3> &translation, const BoundaryConditions &bc) {
    this->position += translation;
    this->position += bc.getCorrection(this->position);
}

void Shape::rotate(const Matrix<3, 3> &rotation) {
    this->orientation = rotation * this->orientation;
}
