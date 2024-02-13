//
// Created by Piotr Kubala on 26/03/2021.
//

#ifndef RAMPACK_MOCKSHAPETRAITS_H
#define RAMPACK_MOCKSHAPETRAITS_H

#include <catch2/trompeloeil.hpp>

#include "core/ShapeTraits.h"
#include "core/Interaction.h"


class MockShapeTraits : public ShapeTraits, public Interaction, public ShapeGeometry, public ShapeDataManager {
public:
    // ShapeTraits methods
    [[nodiscard]] const Interaction &getInteraction() const override { return *this; };
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; };
    [[nodiscard]] const ShapeDataManager &getDataManager() const override { return *this; }

    MAKE_CONST_MOCK2(getPrinter, std::shared_ptr<const ShapePrinter>(const std::string &,
        const std::map<std::string, std::string> &),
                     override);

    // Interaction methods
    MAKE_CONST_MOCK0(hasHardPart, bool(), override);
    MAKE_CONST_MOCK0(hasSoftPart, bool(), override);
    MAKE_CONST_MOCK0(hasWallPart, bool(), override);
    MAKE_CONST_MOCK0(isConvex, bool(), override);
    MAKE_CONST_MOCK9(calculateEnergyBetween, double(const Vector<3> &, const Matrix<3, 3> &, const std::byte *,
                                                    std::size_t, const Vector<3> &, const Matrix<3, 3> &,
                                                    const std::byte *, std::size_t,
                                                    const BoundaryConditions &),
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

    // ShapeGeometry methods
    MAKE_CONST_MOCK1(getVolume, double(const Shape &), override);
    MAKE_CONST_MOCK1(getPrimaryAxis, Vector<3>(const Shape &), override);
    MAKE_CONST_MOCK1(getSecondaryAxis, Vector<3>(const Shape &), override);
    MAKE_CONST_MOCK1(getGeometricOrigin, Vector<3>(const Shape &), override);

    // ShapeDataManager methods
    MAKE_CONST_MOCK0(getShapeDataSize, std::size_t(), override);
    MAKE_CONST_MOCK1(validateShapeData, void(const ShapeData &), override);
    MAKE_CONST_MOCK1(serialize, TextualShapeData (const ShapeData &), override);
    MAKE_CONST_MOCK1(deserialize, ShapeData(const TextualShapeData &), override);
};

#endif //RAMPACK_MOCKSHAPETRAITS_H
