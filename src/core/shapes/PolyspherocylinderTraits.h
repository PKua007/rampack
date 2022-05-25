//
// Created by Piotr Kubala on 26/04/2021.
//

#ifndef RAMPACK_POLYSPHEROCYLINDERTRAITS_H
#define RAMPACK_POLYSPHEROCYLINDERTRAITS_H


#include <ostream>

#include "core/ShapeTraits.h"

/**
 * @brief A class analogous to PolysphereTraits, but for hard spherocylinders.
 */
class PolyspherocylinderTraits : public ShapeTraits, public ShapePrinter, public Interaction {
public:
    /**
     * @brief A single building spherocylinder data.
     * @details Spherocylinder cap centers are given by @a posision +/- @a halfAxis
     */
    struct SpherocylinderData {
        /** @brief Position of the mass centre of the spheroculinder. */
        const Vector<3> position;
        /** @brief The vector joining mass centre with one of the caps. */
        const Vector<3> halfAxis;
        /** @brief The norm of the above vector. */
        const double halfLength{};
        /** @brief Radius of the spherical cap (and also half-width of the spherocylinder. */
        const double radius{};
        /** @brief Spherocylinder's circumsphere radius. */
        const double circumsphereRadius{};

        SpherocylinderData(const Vector<3> &position, const Vector<3> &halfAxis, double radius);

        [[nodiscard]] Vector<3> centreForShape(const Shape &shape) const;
        void toWolfram(std::ostream &out, const Shape &shape) const;
        [[nodiscard]] double getVolume() const;

        /**
         * @brief Returns half-axis for a shape with specific orientation (the orientation matrix is applied to
         * SpherocylinderData::halfAxis).
         */
        [[nodiscard]] Vector<3> halfAxisForShape(const Shape &shape) const;

        friend bool operator==(const SpherocylinderData &lhs, const SpherocylinderData &rhs) {
            return lhs.radius == rhs.radius && lhs.position == rhs.position
                   && (lhs.halfAxis == rhs.halfAxis || lhs.halfAxis == -rhs.halfAxis);
        }

        friend std::ostream &operator<<(std::ostream &os, const SpherocylinderData &data) {
            os << "{pos: " << data.position << ", halfAxis: " << data.halfAxis << ", radius:" << data.radius << "}";
            return os;
        }
    };

    /**
     * @brief A helper class defining a whole particle.
     */
    class PolyspherocylinderGeometry : public ShapeGeometry {
    private:
        std::vector<SpherocylinderData> spherocylinderData;
        Vector<3> primaryAxis;
        Vector<3> secondaryAxis;
        Vector<3> geometricOrigin;

    public:
        PolyspherocylinderGeometry(std::vector<SpherocylinderData> spherocylinderData, const Vector<3> &primaryAxis,
                                   const Vector<3> &secondaryAxis, const Vector<3> &geometricOrigin);

        [[nodiscard]] double getVolume() const override;

        [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const override {
            return shape.getOrientation() * this->primaryAxis;
        }

        [[nodiscard]] Vector<3> getSecondaryAxis(const Shape &shape) const override {
            return shape.getOrientation() * this->secondaryAxis;
        }

        [[nodiscard]] Vector<3> getGeometricOrigin(const Shape &shape) const override {
            return shape.getOrientation() * this->geometricOrigin;
        }

        [[nodiscard]] const std::vector<SpherocylinderData> &getSpherocylinderData() const { return this->spherocylinderData; }

        void normalizeMassCentre();
        [[nodiscard]] Vector<3> calculateMassCentre() const;
        void setGeometricOrigin(const Vector<3> &geometricOrigin_) { this->geometricOrigin = geometricOrigin_; }
    };

private:
    PolyspherocylinderGeometry geometry;

public:
    /**
     * @brief Creates the molecule from a given set of spherocylinders.
     * @param spherocylinderData a set of spherocylinders
     * @param primaryAxis the primary axis of the molecule
     * @param secondaryAxis the secondary axis of the polymer (should be orthogonal to the primary one)
     * @param shouldNormalizeMassCentre if true, the mass centre will be moved to the origin. Otherwise, no translation
     * is applied
     */
    explicit PolyspherocylinderTraits(PolyspherocylinderGeometry geometry) : geometry{std::move(geometry)} { }

    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }
    [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                      const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                      const BoundaryConditions &bc) const override;
    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const override;
    [[nodiscard]] double getRangeRadius() const override;

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return this->geometry; }
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }
    [[nodiscard]] const std::vector<SpherocylinderData> &getSpherocylinderData() const {
        return this->geometry.getSpherocylinderData();
    }

    [[nodiscard]] std::string toWolfram(const Shape &shape) const override;
};


#endif //RAMPACK_POLYSPHEROCYLINDERTRAITS_H
