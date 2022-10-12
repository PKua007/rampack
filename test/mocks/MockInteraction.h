//
// Created by pkua on 05.03.2022.
//

#ifndef RAMPACK_MOCKINTERACTION_H
#define RAMPACK_MOCKINTERACTION_H

#include <catch2/trompeloeil.hpp>

#include "core/Interaction.h"

class MockInteraction : public Interaction {
public:
    MAKE_CONST_MOCK0(hasHardPart, bool(), override);
    MAKE_CONST_MOCK0(hasSoftPart, bool(), override);
    MAKE_CONST_MOCK0(hasWallPart, bool(), override);
    MAKE_CONST_MOCK0(isConvex, bool(), override);
    MAKE_CONST_MOCK7(calculateEnergyBetween, double(const Vector<3> &, const Matrix<3, 3> &, std::size_t,
                                                    const Vector<3> &, const Matrix<3, 3> &, std::size_t,
                                                    const BoundaryConditions &),
                     override);
    MAKE_CONST_MOCK7(overlapBetween, bool(const Vector<3> &, const Matrix<3, 3> &, std::size_t, const Vector<3> &,
                                          const Matrix<3, 3> &, std::size_t, const BoundaryConditions &),
                     override);
    MAKE_CONST_MOCK5(overlapWithWall, bool(const Vector<3> &, const Matrix<3, 3> &, std::size_t, const Vector<3> &,
                                           const Vector<3> &),
                     override);
    MAKE_CONST_MOCK0(getRangeRadius, double(), override);
    MAKE_CONST_MOCK0(getInteractionCentres, std::vector<Vector<3>>(), override);
    MAKE_CONST_MOCK0(getTotalRangeRadius, double(), override);
};


#endif //RAMPACK_MOCKINTERACTION_H
