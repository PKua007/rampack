//
// Created by Piotr Kubala on 13/12/2020.
//

#ifndef RAMPACK_FREEBOUNDARYCONDITIONS_H
#define RAMPACK_FREEBOUNDARYCONDITIONS_H

#include "BoundaryConditions.h"

class FreeBoundaryConditions : public BoundaryConditions {
public:
    [[nodiscard]] std::array<double, 3> getCorrection(const std::array<double, 3> &position) const override;

    [[nodiscard]] std::array<double, 3> getTranslation(const std::array<double, 3> &position1,
                                                       const std::array<double, 3> &position2) const override;

    [[nodiscard]] double getDistance2(const std::array<double, 3> &position1,
                                      const std::array<double, 3> &position2) const override;
};

#endif //RAMPACK_FREEBOUNDARYCONDITIONS_H
