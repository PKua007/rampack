//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_BOUNDARYCONDITIONS_H
#define RAMPACK_BOUNDARYCONDITIONS_H

#include <array>

#include "geometry/Vector.h"

/**
 * @brief A class representing boundary conditions
 */
class BoundaryConditions {
public:
    virtual ~BoundaryConditions() = default;

    /**
     * @brief Sets the dimensions of the box which BoundaryConditions should apply to
     */
    virtual void setLinearSize(const std::array<double, 3> &size) = 0;

    /**
     * @brief For a given @a position, it returns the vector, which should be added to it in order to fit @a position
     * within the box
     */
    [[nodiscard]] virtual Vector<3> getCorrection(const Vector<3> &position) const = 0;

    /**
     * @brief For given two vectors @a position1 and @a position2, it returns the vector, which should be added to
     * @a position2 to move it near @a position1 according to BC
     */
    [[nodiscard]] virtual Vector<3> getTranslation(const Vector<3> &position1, const Vector<3> &position2) const = 0;

    /**
     * @brief Returns the closest distance squared between @a position1 and @a position2 according to BC
     */
    [[nodiscard]] virtual double getDistance2(const Vector<3> &position1, const Vector<3> &position2) const = 0;
};

#endif //RAMPACK_BOUNDARYCONDITIONS_H
