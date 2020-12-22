//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SPHERETRAITS_H
#define RAMPACK_SPHERETRAITS_H

#include <variant>

#include "core/ShapeTraits.h"
#include "core/CentralInteraction.h"

class SphereTraits : public ShapeTraits, public ShapePrinter, public Interaction {
public:
    class HardInteraction { };

private:
    double radius{};
    std::unique_ptr<CentralInteraction> interaction{};

public:
    explicit SphereTraits(double radius = 1);
    SphereTraits(double radius, std::unique_ptr<CentralInteraction> centralInteraction);

    [[nodiscard]] const Interaction &getInteraction() const override;
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }
    [[nodiscard]] double getVolume() const override;

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

    [[nodiscard]] std::string toWolfram(const Shape &shape, double scale) const override;

    [[nodiscard]] double getRadius() const { return this->radius; }
};


#endif //RAMPACK_SPHERETRAITS_H
