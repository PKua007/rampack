//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_PERIODICBOUNDARYCONDITIONS_H
#define RAMPACK_PERIODICBOUNDARYCONDITIONS_H

#include "BoundaryConditions.h"

class PeriodicBoundaryConditions : public BoundaryConditions {
private:
    std::array<double, 3> size{};

public:
    PeriodicBoundaryConditions() : size{1, 1, 1} { }
    explicit PeriodicBoundaryConditions(const std::array<double, 3> &size);
    explicit PeriodicBoundaryConditions(double linearSize);

    void setLinearSize(const std::array<double, 3> &size_) override;
    [[nodiscard]] Vector<3> getCorrection(const Vector<3> &position) const override;
    [[nodiscard]] Vector<3> getTranslation(const Vector<3> &position1, const Vector<3> &position2) const override;
    [[nodiscard]] double getDistance2(const Vector<3> &position1, const Vector<3> &position2) const override;
};


#endif //RAMPACK_PERIODICBOUNDARYCONDITIONS_H
