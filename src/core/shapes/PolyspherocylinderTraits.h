//
// Created by Piotr Kubala on 26/04/2021.
//

#ifndef RAMPACK_POLYSPHEROCYLINDERTRAITS_H
#define RAMPACK_POLYSPHEROCYLINDERTRAITS_H


#include <ostream>
#include <map>

#include "core/ShapeTraits.h"
#include "geometry/xenocollide/AbstractXCGeometry.h"
#include "OptionalAxis.h"


/**
 * @brief A class analogous to PolysphereTraits, but for hard spherocylinders.
 */
class PolyspherocylinderTraits : public ShapeTraits, public Interaction {
public:
    /**
     * @brief A single building spherocylinder data.
     * @details Spherocylinder cap centers are given by @a posision +/- @a halfAxis
     */
    struct SpherocylinderData {
        /** @brief Position of the mass centre of the spherocylinder. */
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
     * @details The class, apart from standard named points (see ShapeGeometry::getNamedPoint()) and
     * @a customNamedPoints from the constructor, defines points "ox", "bx" and "ex" representing, respectively,
     * origin, beginning cap center and end cap center of constituent spherocylinders, where "x" is
     * spherocylinder's index starting from 0.
     */
    class PolyspherocylinderGeometry : public ShapeGeometry {
    private:
        std::vector<SpherocylinderData> spherocylinderData;
        std::optional<Vector<3>> primaryAxis;
        std::optional<Vector<3>> secondaryAxis;
        Vector<3> geometricOrigin;
        double volume{};

        [[nodiscard]] double calculateVolume() const;

    public:
        /**
         * @brief Constructs the object.
         * @param spherocylinderData set of spherocylinders
         * @param primaryAxis the primary axis of the molecule
         * @param secondaryAxis the secondary axis of the polymer (should be orthogonal to the primary one)
         * @param geometricOrigin geometric origin of the molecule which can be different that the mass centre
         * @param volume volume of the shape
         * @param customNamedPoints custom named points in addition to default ones (see
         * PolyspherocylinderGeometry::getNamedPoint)
         */
        PolyspherocylinderGeometry(std::vector<SpherocylinderData> spherocylinderData, OptionalAxis primaryAxis,
                                   OptionalAxis secondaryAxis, const Vector<3> &geometricOrigin = {0, 0, 0},
                                   std::optional<double> volume = 0,
                                   const ShapeGeometry::NamedPoints& customNamedPoints = {});

        [[nodiscard]] double getVolume() const override { return this->volume; }

        [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const override {
            if (!this->primaryAxis.has_value())
                throw std::runtime_error("PolyspherocylinderGeometry::getPrimaryAxis: primary axis not defined");
            return shape.getOrientation() * this->primaryAxis.value();
        }

        [[nodiscard]] Vector<3> getSecondaryAxis(const Shape &shape) const override {
            if (!this->primaryAxis.has_value())
                throw std::runtime_error("PolyspherocylinderGeometry::getSecondaryAxis: secondary axis not defined");
            return shape.getOrientation() * this->secondaryAxis.value();
        }

        [[nodiscard]] Vector<3> getGeometricOrigin(const Shape &shape) const override {
            return shape.getOrientation() * this->geometricOrigin;
        }

        [[nodiscard]] const std::vector<SpherocylinderData> &getSpherocylinderData() const {
            return this->spherocylinderData;
        }

        [[nodiscard]] bool spherocylindersOverlap() const;

        void setGeometricOrigin(const Vector<3> &geometricOrigin_) { this->geometricOrigin = geometricOrigin_; }

        void addCustomNamedPoints(const ShapeGeometry::NamedPoints &namedPoints) {
            this->registerNamedPoints(namedPoints);
        }
    };

private:
    class WolframPrinter : public ShapePrinter {
    private:
        const PolyspherocylinderTraits &traits;

    public:
        explicit WolframPrinter( const PolyspherocylinderTraits &traits) : traits{traits} { }

        [[nodiscard]] std::string print(const Shape &shape) const override;
    };

    static std::shared_ptr<AbstractXCGeometry> buildXCSpherocylinder(const SpherocylinderData &scData);

    [[nodiscard]] std::shared_ptr<ShapePrinter> createObjPrinter(std::size_t subdivisions) const;

    PolyspherocylinderGeometry geometry;
    std::shared_ptr<WolframPrinter> wolframPrinter;

public:
    /** @brief The default number of sphere subdivisions when printing the shape (see XCPrinter::XCPrinter
     * @a subdivision parameter) */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 3;

    /**
     * @brief Creates the molecule from a given set of spherocylinders.
     * @param geometry PolyspherocylinderGeometry describing the shape
     */
    explicit PolyspherocylinderTraits(PolyspherocylinderGeometry geometry);

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

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return this->geometry; }

    /**
     * @brief Returns ShapePrinter for a given @a format.
     * @details The following formats are supported:
     * <ol>
     *     <li> `wolfram` - Wolfram Mathematica shape
     *     <li> `obj` - Wavefront OBJ triangle mesh (it accepts @a mesh_divisions parameter, default: 3)
     * </ol>
     */
    [[nodiscard]] std::shared_ptr<const ShapePrinter>
    getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const override;

    [[nodiscard]] const std::vector<SpherocylinderData> &getSpherocylinderData() const {
        return this->geometry.getSpherocylinderData();
    }
};


#endif //RAMPACK_POLYSPHEROCYLINDERTRAITS_H
