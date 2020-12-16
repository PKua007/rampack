//
// Created by Piotr Kubala on 13/12/2020.
//

#ifndef RAMPACK_FREEBOUNDARYCONDITIONS_H
#define RAMPACK_FREEBOUNDARYCONDITIONS_H

#include "BoundaryConditions.h"

class FreeBoundaryConditions : public BoundaryConditions {
public:
    [[nodiscard]] Vector<3> getCorrection([[maybe_unused]] const Vector<3> &position) const override {
        return Vector<3>{};
    }

    [[nodiscard]] Vector<3> getTranslation([[maybe_unused]] const Vector<3> &position1,
                                           [[maybe_unused]] const Vector<3> &position2) const override
    {
        return Vector<3>{};
    }

    [[nodiscard]] double getDistance2([[maybe_unused]] const Vector<3> &position1,
                                      [[maybe_unused]] const Vector<3> &position2) const override
    {
        return (position2 - position1).norm2();
    }
};

#endif //RAMPACK_FREEBOUNDARYCONDITIONS_H
