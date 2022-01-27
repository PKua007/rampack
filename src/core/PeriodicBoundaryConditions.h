//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_PERIODICBOUNDARYCONDITIONS_H
#define RAMPACK_PERIODICBOUNDARYCONDITIONS_H

#include "BoundaryConditions.h"

/**
 * @brief Periodic boundary conditions on cuboidal box.
 */
class PeriodicBoundaryConditions : public BoundaryConditions {
private:
    std::array<double, 3> size{};

public:
    /**
     * @brief A default constructor creating PBC for 1 x 1 x 1 cube.
     */
    PeriodicBoundaryConditions() : size{1, 1, 1} { }

    /**
     * @brief A constructor creating PBC for cuboidal box with side lengths given by @a size.
     */
    explicit PeriodicBoundaryConditions(const std::array<double, 3> &size);

    /**
     * @brief A constructor creating PBC for cubic box of side length @a linearSize
     */
    explicit PeriodicBoundaryConditions(double linearSize);

    void setLinearSize(const std::array<double, 3> &size_) override;
    [[nodiscard]] Vector<3> getCorrection(const Vector<3> &position) const override;
    [[nodiscard]] Vector<3> getTranslation(const Vector<3> &position1, const Vector<3> &position2) const override;
    [[nodiscard]] double getDistance2(const Vector<3> &position1, const Vector<3> &position2) const override;
};


#endif //RAMPACK_PERIODICBOUNDARYCONDITIONS_H
