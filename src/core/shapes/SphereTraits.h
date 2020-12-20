//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SPHERETRAITS_H
#define RAMPACK_SPHERETRAITS_H

#include "core/ShapeTraits.h"

class SphereTraits : public ShapeTraits, public ShapePrinter, public Interaction {
private:
    double radius{};

public:
    SphereTraits() : radius{1} { }
    explicit SphereTraits(double radius);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }

    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }
    [[nodiscard]] double calculateEnergyBetween([[maybe_unused]] const Shape &shape1,
                                                [[maybe_unused]] const Shape &shape2, [[maybe_unused]] double scale,
                                                [[maybe_unused]] const BoundaryConditions &bc) const override
    {
        return 0;
    }

    [[nodiscard]] bool overlapBetween(const Shape &shape1, const Shape &shape2, double scale,
                                      const BoundaryConditions &bc) const override;

    [[nodiscard]] double getVolume() const override;
    [[nodiscard]] std::string toWolfram(const Shape &shape, double scale) const override;

    [[nodiscard]] double getRadius() const { return this->radius; }
};


#endif //RAMPACK_SPHERETRAITS_H
