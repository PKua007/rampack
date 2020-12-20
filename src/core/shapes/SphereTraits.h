//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SPHERETRAITS_H
#define RAMPACK_SPHERETRAITS_H

#include <variant>

#include "core/ShapeTraits.h"

class SphereTraits : public ShapeTraits, public ShapePrinter, public Interaction {
public:
    struct HardInteraction { };
    struct LennardJonesInteraction {
    private:
        double epsilon{};
        double sigma{};

        friend class SphereTraits;

    public:
        LennardJonesInteraction(double epsilon, double sigma);
    };

private:
    double radius{};
    std::variant<HardInteraction, LennardJonesInteraction> interaction{};

public:
    explicit SphereTraits(double radius = 1);
    SphereTraits(double radius, HardInteraction hardInteraction);
    SphereTraits(double radius, LennardJonesInteraction lennardJonesInteraction);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }

    [[nodiscard]] bool hasHardPart() const override {
        return std::holds_alternative<HardInteraction>(this->interaction);
    }

    [[nodiscard]] bool hasSoftPart() const override {
        return std::holds_alternative<LennardJonesInteraction>(this->interaction);
    }

    [[nodiscard]] double calculateEnergyBetween(const Shape &shape1, const Shape &shape2, double scale,
                                                const BoundaryConditions &bc) const override;
    [[nodiscard]] bool overlapBetween(const Shape &shape1, const Shape &shape2, double scale,
                                      const BoundaryConditions &bc) const override;

    [[nodiscard]] double getVolume() const override;
    [[nodiscard]] std::string toWolfram(const Shape &shape, double scale) const override;

    [[nodiscard]] double getRadius() const { return this->radius; }
};


#endif //RAMPACK_SPHERETRAITS_H
