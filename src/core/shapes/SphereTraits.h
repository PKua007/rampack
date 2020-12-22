//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SPHERETRAITS_H
#define RAMPACK_SPHERETRAITS_H

#include <variant>

#include "core/ShapeTraits.h"
#include "core/CentralInteraction.h"

class SphereTraits : public ShapeTraits, public ShapePrinter {
private:
    class HardInteraction : public Interaction {
    private:
        double radius{};

    public:
        explicit HardInteraction(double radius) : radius(radius) { }

        [[nodiscard]] bool hasHardPart() const override { return true; }
        [[nodiscard]] bool hasSoftPart() const override { return false; }
        [[nodiscard]] bool overlapBetween(const Shape &shape1, const Shape &shape2, double scale,
                                          const BoundaryConditions &bc) const override;
    };

    double radius{};
    std::unique_ptr<Interaction> interaction{};

public:
    explicit SphereTraits(double radius = 1);
    SphereTraits(double radius, std::unique_ptr<CentralInteraction> centralInteraction);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this->interaction; };
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }
    [[nodiscard]] double getVolume() const override;

    [[nodiscard]] std::string toWolfram(const Shape &shape, double scale) const override;

    [[nodiscard]] double getRadius() const { return this->radius; }
};


#endif //RAMPACK_SPHERETRAITS_H
