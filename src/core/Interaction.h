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
    [[nodiscard]] virtual double calculateEnergyBetween([[maybe_unused]] const Shape &shape1,
                                                        [[maybe_unused]] const Shape &shape2,
                                                        [[maybe_unused]] const BoundaryConditions &bc) const
    {
        return 0;
    }

    [[nodiscard]] virtual bool overlapBetween([[maybe_unused]] const Shape &shape1,
                                              [[maybe_unused]] const Shape &shape2,
                                              [[maybe_unused]] const BoundaryConditions &bc) const
    {
        return false;
    }

    [[nodiscard]] virtual double getRangeRadius() const { return std::numeric_limits<double>::infinity(); }
};


#endif //RAMPACK_INTERACTION_H
