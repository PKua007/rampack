//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_BOUNDARYCONDITIONS_H
#define RAMPACK_BOUNDARYCONDITIONS_H

#include <array>

class BoundaryConditions {
public:
    virtual ~BoundaryConditions() = default;

    [[nodiscard]] virtual std::array<double, 3> getCorrection(const std::array<double, 3> &position) const = 0;
    [[nodiscard]] virtual std::array<double, 3> getTranslation(const std::array<double, 3> &position1,
                                                               const std::array<double, 3> &position2) const = 0;
    [[nodiscard]] virtual double getDistance2(const std::array<double, 3> &position1,
                                              const std::array<double, 3> &position2) const = 0;
};

#endif //RAMPACK_BOUNDARYCONDITIONS_H
