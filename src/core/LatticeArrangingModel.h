//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_LATTICEARRANGINGMODEL_H
#define RAMPACK_LATTICEARRANGINGMODEL_H

#include <vector>
#include <memory>

#include "Shape.h"
#include "BoundaryConditions.h"

class LatticeArrangingModel {
public:
    [[nodiscard]] std::vector<std::unique_ptr<Shape>> arrange(std::size_t numOfParticles, double linearSize) const;
};


#endif //RAMPACK_LATTICEARRANGINGMODEL_H
