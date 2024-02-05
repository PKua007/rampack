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
    MAKE_CONST_MOCK9(calculateEnergyBetween, double(const Vector<3> &, const Matrix<3, 3> &, const std::byte *,
                                                    std::size_t, const Vector<3> &, const Matrix<3, 3> &,
                                                    const std::byte *, std::size_t, const BoundaryConditions &),
                     override);
    MAKE_CONST_MOCK9(overlapBetween, bool(const Vector<3> &, const Matrix<3, 3> &, const std::byte *, std::size_t,
                                          const Vector<3> &, const Matrix<3, 3> &, const std::byte *, std::size_t,
                                          const BoundaryConditions &),
                     override);
    MAKE_CONST_MOCK6(overlapWithWall, bool(const Vector<3> &, const Matrix<3, 3> &, const std::byte *, std::size_t,
                                           const Vector<3> &, const Vector<3> &),
                     override);
    MAKE_CONST_MOCK1(getRangeRadius, double(const std::byte *), override);
    MAKE_CONST_MOCK1(getInteractionCentres, std::vector<Vector<3>>(const std::byte *), override);
    MAKE_CONST_MOCK1(getTotalRangeRadius, double(const std::byte *), override);
};


#endif //RAMPACK_MOCKINTERACTION_H
