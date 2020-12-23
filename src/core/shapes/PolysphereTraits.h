//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_POLYSPHERETRAITS_H
#define RAMPACK_POLYSPHERETRAITS_H

#include <utility>

#include "core/ShapeTraits.h"
#include "core/interactions/CentralInteraction.h"

class PolysphereTraits : public ShapeTraits, public ShapePrinter {
public:
    struct SphereData {
        const Vector<3> position;
        const double radius{};

        SphereData(const Vector<3> &position, double radius);

        [[nodiscard]] Vector<3> centreForShape(const Shape &shape, double scale) const;
        void toWolfram(std::ostream &out, const Shape &shape, double scale) const;
    };

private:
    class HardInteraction : public Interaction {
    private:
        std::vector<SphereData> sphereData;

    public:
        explicit HardInteraction(std::vector<SphereData> sphereData) : sphereData{std::move(sphereData)} { }

        [[nodiscard]] bool hasHardPart() const override { return true; }
        [[nodiscard]] bool hasSoftPart() const override { return false; }
        [[nodiscard]] bool overlapBetween(const Shape &shape1, const Shape &shape2, double scale,
                                          const BoundaryConditions &bc) const override;
    };

    std::vector<SphereData> sphereData;
    std::unique_ptr<Interaction> interaction{};

public:
    explicit PolysphereTraits(const std::vector<SphereData> &sphereData);
    PolysphereTraits(std::vector<SphereData> sphereData, std::unique_ptr<CentralInteraction> centralInteraction);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this->interaction; }
    [[nodiscard]] double getVolume() const override;
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }

    [[nodiscard]] std::string toWolfram(const Shape &shape, double scale) const override;

    [[nodiscard]] const std::vector<SphereData> &getSphereData() const { return this->sphereData; }
};


#endif //RAMPACK_POLYSPHERETRAITS_H
