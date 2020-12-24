//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_BOUNDARYCONDITIONS_H
#define RAMPACK_BOUNDARYCONDITIONS_H

#include "geometry/Vector.h"

class BoundaryConditions {
public:
    virtual ~BoundaryConditions() = default;

    virtual void setLinearSize(double size) = 0;
    [[nodiscard]] virtual Vector<3> getCorrection(const Vector<3> &position) const = 0;
    [[nodiscard]] virtual Vector<3> getTranslation(const Vector<3> &position1, const Vector<3> &position2) const = 0;
    [[nodiscard]] virtual double getDistance2(const Vector<3> &position1, const Vector<3> &position2) const = 0;
};

#endif //RAMPACK_BOUNDARYCONDITIONS_H
