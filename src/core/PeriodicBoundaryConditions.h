//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_PERIODICBOUNDARYCONDITIONS_H
#define RAMPACK_PERIODICBOUNDARYCONDITIONS_H

#include "BoundaryConditions.h"

class PeriodicBoundaryConditions : public BoundaryConditions {
private:
    double linearSize{};

public:
    PeriodicBoundaryConditions() : linearSize{1} { }
    explicit PeriodicBoundaryConditions(double linearSize);

    [[nodiscard]] Vector<3> getCorrection(const Vector<3> &position) const override;
    [[nodiscard]] Vector<3> getTranslation(const Vector<3> &position1, const Vector<3> &position2) const override;
    [[nodiscard]] double getDistance2(const Vector<3> &position1, const Vector<3> &position2) const override;
};


#endif //RAMPACK_PERIODICBOUNDARYCONDITIONS_H
