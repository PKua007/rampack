//
// Created by Piotr Kubala on 26/03/2021.
//

#ifndef RAMPACK_MOCKSHAPETRAITS_H
#define RAMPACK_MOCKSHAPETRAITS_H

#include <catch2/trompeloeil.hpp>

#include "core/ShapeTraits.h"
#include "core/Interaction.h"

class MockShapeTraits : public ShapeTraits, public Interaction {
public:
    [[nodiscard]] const Interaction &getInteraction() const override { return *this; };
    MAKE_CONST_MOCK0(getVolume, double(), override);
    MAKE_CONST_MOCK1(getPrimaryAxis, Vector<3>(const Shape &), override);
    MAKE_CONST_MOCK0(getPrinter, const ShapePrinter &(), override);
    MAKE_CONST_MOCK0(hasHardPart, bool(), override);
    MAKE_CONST_MOCK0(hasSoftPart, bool(), override);
    MAKE_CONST_MOCK7(calculateEnergyBetween, double(const Vector<3> &, const Matrix<3, 3> &, std::size_t,
                                                    const Vector<3> &, const Matrix<3, 3> &, std::size_t,
                                                    const BoundaryConditions &),
                     override);
    MAKE_CONST_MOCK7(overlapBetween, bool(const Vector<3> &, const Matrix<3, 3> &, std::size_t, const Vector<3> &,
                                          const Matrix<3, 3> &, std::size_t, const BoundaryConditions &),
                     override);
    MAKE_CONST_MOCK0(getRangeRadius, double(), override);
    MAKE_CONST_MOCK0(getInteractionCentres, std::vector<Vector<3>>(), override);
    MAKE_CONST_MOCK0(getTotalRangeRadius, double(), override);
};

#endif //RAMPACK_MOCKSHAPETRAITS_H
