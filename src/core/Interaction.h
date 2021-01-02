//
// Created by Piotr Kubala on 19/12/2020.
//

#ifndef RAMPACK_INTERACTION_H
#define RAMPACK_INTERACTION_H

#include <vector>

#include "Shape.h"
#include "BoundaryConditions.h"

class Interaction {
public:
    virtual ~Interaction() = default;

    [[nodiscard]] virtual bool hasHardPart() const = 0;
    [[nodiscard]] virtual bool hasSoftPart() const = 0;
    [[nodiscard]] virtual double calculateEnergyBetween([[maybe_unused]] const Vector<3> &pos1,
                                                        [[maybe_unused]] const Matrix<3, 3> &orientation1,
                                                        [[maybe_unused]] std::size_t idx1,
                                                        [[maybe_unused]] const Vector<3> &pos2,
                                                        [[maybe_unused]] const Matrix<3, 3> &orientation2,
                                                        [[maybe_unused]] std::size_t idx2,
                                                        [[maybe_unused]] const BoundaryConditions &bc) const
    {
        return 0;
    }

    [[nodiscard]] virtual bool overlapBetween([[maybe_unused]] const Vector<3> &pos1,
                                              [[maybe_unused]] const Matrix<3, 3> &orientation1,
                                              [[maybe_unused]] std::size_t idx1,
                                              [[maybe_unused]] const Vector<3> &pos2,
                                              [[maybe_unused]] const Matrix<3, 3> &orientation2,
                                              [[maybe_unused]] std::size_t idx2,
                                              [[maybe_unused]] const BoundaryConditions &bc) const
    {
        return false;
    }

    [[nodiscard]] virtual double getRangeRadius() const { return std::numeric_limits<double>::infinity(); }

    [[nodiscard]] virtual std::vector<Vector<3>> getInteractionCentres() const { return {}; }
};


#endif //RAMPACK_INTERACTION_H
