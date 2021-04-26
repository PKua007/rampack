//
// Created by Piotr Kubala on 26/04/2021.
//

#ifndef RAMPACK_POLYSPHEROCYLINDERTRAITS_H
#define RAMPACK_POLYSPHEROCYLINDERTRAITS_H


#include <utility>
#include <ostream>

#include "core/ShapeTraits.h"

class PolyspherocylinderTraits : public ShapeTraits, public ShapePrinter, Interaction {
public:
    struct SpherocylinderData {
        const Vector<3> position;
        const Vector<3> halfAxis;
        const double halfLength{};
        const double radius{};
        const double circumsphereRadius{};
        const double maxDistance{};

        SpherocylinderData(const Vector<3> &position, const Vector<3> &halfAxis, double radius);

        [[nodiscard]] Vector<3> centreForShape(const Shape &shape) const;
        void toWolfram(std::ostream &out, const Shape &shape) const;
        [[nodiscard]] double getVolume() const;
        [[nodiscard]] Vector<3> halfAxisForShape(const Shape &shape) const;

        friend bool operator==(const SpherocylinderData &lhs, const SpherocylinderData &rhs) {
            return lhs.radius == rhs.radius && lhs.position == rhs.position
                   &&(lhs.halfAxis == rhs.halfAxis || lhs.halfAxis == -rhs.halfAxis);
        }

        friend std::ostream &operator<<(std::ostream &os, const SpherocylinderData &data) {
            os << "{pos: " << data.position << ", halfAxis: " << data.halfAxis << ", radius:" << data.radius << "}";
            return os;
        }
    };

private:
    static constexpr double EPSILON = 0.0000000001;

    std::vector<SpherocylinderData> spherocylinderData;
    std::unique_ptr<Interaction> interaction{};
    Vector<3> primaryAxis;

    static double distance2Between(const Vector<3> &capCentre11, const Vector<3> &capCentre12,
                                   const Vector<3> &capCentre21, const Vector<3> &capCentre22);

    void normalizeMassCentre();

public:
    PolyspherocylinderTraits(const std::vector<SpherocylinderData> &spherocylinderData, const Vector<3> &primaryAxis,
                             bool shouldNormalizeMassCentre);

    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }
    [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                      const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                      const BoundaryConditions &bc) const override;
    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const override;
    [[nodiscard]] double getRangeRadius() const override;

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] double getVolume() const override;
    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const override;
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }

    [[nodiscard]] std::string toWolfram(const Shape &shape) const override;

    [[nodiscard]] const std::vector<SpherocylinderData> &getSpherocylinderData() const {
        return this->spherocylinderData;
    }
};


#endif //RAMPACK_POLYSPHEROCYLINDERTRAITS_H
