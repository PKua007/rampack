//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_POLYSPHERETRAITS_H
#define RAMPACK_POLYSPHERETRAITS_H

#include <utility>
#include <ostream>
#include <map>

#include "core/ShapeTraits.h"
#include "core/interactions/CentralInteraction.h"


/**
 * @brief A polymer consisting of identical or different hard of soft-interacting spheres.
 */
class PolysphereTraits : public ShapeTraits, public ShapePrinter {
public:
    /**
     * @brief A helper class describing a single spherical bead.
     */
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

    /**
     * @brief A helper class defining a whole particle.
     * @details The class, apart from standard named points (see ShapeGeometry::getNamedPoint()) and
     * @a customNamedPoints from the constructor, defines points "sx" representing constituent spheres, where "x" is
     * sphere's index starting from 0.
     */
    class PolysphereGeometry : public ShapeGeometry {
    private:
        std::vector<SphereData> sphereData;
        Vector<3> primaryAxis;
        Vector<3> secondaryAxis;
        Vector<3> geometricOrigin;

    public:
        /**
         * @brief Constructs the object.
         * @param sphereData sphere data describing all constituent monomers
         * @param primaryAxis the primary axis of the polymer
         * @param secondaryAxis the secondary axis of the polymer (should be orthogonal to the primary one)
         * @param geometricOrigin geometric origin of the molecule which can be different that the mass centre
         * @param customNamedPoints custom named points in addition to default ones (see
         * PolysphereGeometry::getNamedPoint)
         */
        PolysphereGeometry(std::vector<SphereData> sphereData, const Vector<3> &primaryAxis,
                           const Vector<3> &secondaryAxis, const Vector<3> &geometricOrigin = {0, 0, 0},
                           const ShapeGeometry::NamedPoints& customNamedPoints = {});

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

        [[nodiscard]] const std::vector<SphereData> &getSphereData() const { return this->sphereData; }

        /**
         * @brief Calculates mass centre and moves it to {0, 0, 0} (geometric origin is moved accordingly).
         * @details Sphere overlaps are not accounted for.
         */
        void normalizeMassCentre();

        /**
         * @brief Calculates and returns mass centre.
         * @details Sphere overlaps are not accounted for.
         */
        [[nodiscard]] Vector<3> calculateMassCentre() const;

        void setGeometricOrigin(const Vector<3> &geometricOrigin_) { this->geometricOrigin = geometricOrigin_; }

        void addCustomNamedPoints(const ShapeGeometry::NamedPoints &namedPoints) {
            this->registerNamedPoints(namedPoints);
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
        [[nodiscard]] bool hasWallPart() const override { return true; }
        [[nodiscard]] bool isConvex() const override { return false; }
        [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                          const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                          const BoundaryConditions &bc) const override;
        [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, std::size_t idx,
                                           const Vector<3> &wallOrigin, const Vector<3> &wallVector) const override;

        [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const override;

        [[nodiscard]] double getRangeRadius() const override;
    };

    PolysphereGeometry geometry;
    std::unique_ptr<Interaction> interaction{};

public:
    /**
     * @brief Construct the polymer from the specified @a sphereData.
     * @param geometry PolysphereGeometry describing the molecule.
     */
    explicit PolysphereTraits(PolysphereGeometry geometry);

    /**
     * @brief Similar as PolysphereTraits::PolysphereTraits(const std::vector<SphereData> &, const Vector<3> &, bool),
     * but for soft central interaction given by @a centralInteraction.
     */
    PolysphereTraits(PolysphereGeometry geometry, std::unique_ptr<CentralInteraction> centralInteraction);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this->interaction; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return this->geometry; }
    [[nodiscard]] const ShapePrinter &getPrinter(const std::string &format) const override;

    [[nodiscard]] std::string toWolfram(const Shape &shape) const override;

    [[nodiscard]] const std::vector<SphereData> &getSphereData() const { return this->geometry.getSphereData(); }
};


#endif //RAMPACK_POLYSPHERETRAITS_H
