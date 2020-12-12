//
// Created by Piotr Kubala on 12/12/2020.
//

#include <algorithm>
#include <numeric>

#include "Shape.h"

void Shape::translate(const std::array<double, 3> &translation, const BoundaryConditions &bc) {
    std::transform(this->position.begin(), this->position.end(), translation.begin(), this->position.end(),
                   std::plus<>{});
    auto bcCorrection = bc.getCorrection(this->position);
    std::transform(this->position.begin(), this->position.end(), bcCorrection.begin(), this->position.end(),
                   std::plus<>{});
}
