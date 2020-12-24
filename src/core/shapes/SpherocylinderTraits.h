//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SPHEROCYLINDERTRAITS_H
#define RAMPACK_SPHEROCYLINDERTRAITS_H

#include "core/ShapeTraits.h"

class SpherocylinderTraits : public ShapeTraits, public ShapePrinter, public Interaction {
private:
    double length{};    // distance between two spherical caps centres
    double radius{};    // radius of spherical caps

    static constexpr double EPSILON = 0.0000000001;

    [[nodiscard]] Vector<3> getCapCentre(short beginOrEnd, const Shape &shape) const;
    [[nodiscard]] double distance2Between(const Shape &shape1, const Shape &shape2) const;

public:
    SpherocylinderTraits() : length{1}, radius{1} { }
    SpherocylinderTraits(double length, double radius);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }

    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }
    [[nodiscard]] bool overlapBetween(const Shape &shape1, const Shape &shape2,
                                      const BoundaryConditions &bc) const override;

    [[nodiscard]] double getVolume() const override;
    [[nodiscard]] std::string toWolfram(const Shape &shape) const override;
};


#endif //RAMPACK_SPHEROCYLINDERTRAITS_H
