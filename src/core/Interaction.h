//
// Created by Piotr Kubala on 19/12/2020.
//

#ifndef RAMPACK_INTERACTION_H
#define RAMPACK_INTERACTION_H

#include "Shape.h"
#include "BoundaryConditions.h"

class Interaction {
public:
    virtual ~Interaction() = default;

    [[nodiscard]] virtual bool hasHardPart() const = 0;
    [[nodiscard]] virtual bool hasSoftPart() const = 0;
    [[nodiscard]] virtual double calculateEnergyBetween(const Shape &shape1, const Shape &shape2, double scale,
                                                        const BoundaryConditions &bc) const = 0;
    [[nodiscard]] virtual bool overlapBetween(const Shape &shape1, const Shape &shape2, double scale,
                                              const BoundaryConditions &bc) const = 0;
};


#endif //RAMPACK_INTERACTION_H
