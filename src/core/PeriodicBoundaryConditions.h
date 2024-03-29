//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_PERIODICBOUNDARYCONDITIONS_H
#define RAMPACK_PERIODICBOUNDARYCONDITIONS_H

#include "BoundaryConditions.h"
#include "TriclinicBox.h"

/**
 * @brief Periodic boundary conditions on cuboidal box.
 */
class PeriodicBoundaryConditions : public BoundaryConditions {
private:
    TriclinicBox box;

public:
    /**
     * @brief A default constructor creating PBC for 1 x 1 x 1 cube.
     */
    PeriodicBoundaryConditions() = default;

    explicit PeriodicBoundaryConditions(const TriclinicBox &box);

    /**
     * @brief A constructor creating PBC for cuboidal box with side lengths given by @a size.
     */
    explicit PeriodicBoundaryConditions(const std::array<double, 3> &size)
            : PeriodicBoundaryConditions(TriclinicBox(size))
    { }

    /**
     * @brief A constructor creating PBC for cubic box of side length @a linearSize
     */
    explicit PeriodicBoundaryConditions(double linearSize)
            : PeriodicBoundaryConditions({linearSize, linearSize, linearSize})
    { }

    void setBox(const TriclinicBox &box) override;
    [[nodiscard]] Vector<3> getCorrection(const Vector<3> &position) const override;
    [[nodiscard]] Vector<3> getTranslation(const Vector<3> &position1, const Vector<3> &position2) const override;
    [[nodiscard]] double getDistance2(const Vector<3> &position1, const Vector<3> &position2) const override;
};


#endif //RAMPACK_PERIODICBOUNDARYCONDITIONS_H
