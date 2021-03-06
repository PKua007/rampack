//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_POLYSPHERETRAITS_H
#define RAMPACK_POLYSPHERETRAITS_H

#include <utility>
#include <ostream>

#include "core/ShapeTraits.h"
#include "core/interactions/CentralInteraction.h"

class PolysphereTraits : public ShapeTraits, public ShapePrinter {
public:
    struct SphereData {
        const Vector<3> position;
        const double radius{};

        SphereData(const Vector<3> &position, double radius);

        [[nodiscard]] Vector<3> centreForShape(const Shape &shape) const;
        void toWolfram(std::ostream &out, const Shape &shape) const;

        friend bool operator==(const SphereData &lhs, const SphereData &rhs) {
            return std::tie(lhs.position, lhs.radius) == std::tie(rhs.position, rhs.radius);
        }

        friend std::ostream &operator<<(std::ostream &os, const SphereData &data) {
            return os << "{" << data.position << ", " << data.radius << "}";
        }
    };

private:
    class HardInteraction : public Interaction {
    private:
        std::vector<SphereData> sphereData;

    public:
        explicit HardInteraction(std::vector<SphereData> sphereData);

        [[nodiscard]] bool hasHardPart() const override { return true; }
        [[nodiscard]] bool hasSoftPart() const override { return false; }
        [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                          const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                          const BoundaryConditions &bc) const override;

        [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const override;

        [[nodiscard]] double getRangeRadius() const override;
    };

    std::vector<SphereData> sphereData;
    std::unique_ptr<Interaction> interaction{};
    Vector<3> primaryAxis;

    void normalizeMassCentre();

public:
    explicit PolysphereTraits(const std::vector<SphereData> &sphereData, const Vector<3> &primaryAxis,
                              bool shouldNormalizeMassCentre);
    PolysphereTraits(std::vector<SphereData> sphereData, std::unique_ptr<CentralInteraction> centralInteraction,
                     const Vector<3> &primaryAxis, bool shouldNormalizeMassCentre);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this->interaction; }
    [[nodiscard]] double getVolume() const override;
    Vector<3> getPrimaryAxis(const Shape &shape) const override;
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }

    [[nodiscard]] std::string toWolfram(const Shape &shape) const override;

    [[nodiscard]] const std::vector<SphereData> &getSphereData() const { return this->sphereData; }
};


#endif //RAMPACK_POLYSPHERETRAITS_H
