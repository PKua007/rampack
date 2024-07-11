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
#include "GenericShapeRegistry.h"


/**
 * @brief Class representing a single species of a polyspherocylinder for PolyspherocylinderTraits, conforming to
 * GenericShapeRegistry @a ConcreteSpecies template parameter.
 */
class PolyspherocylinderShape /* : public GenericShapeRegistry::ConcreteSpecies */ {
public:
    /**
     * @brief A single spherocylindrical part of a polyspherocylinder. Spherocylinder cap centers are given by
     * SpherocylinderData::position +/- SpherocylinderData::halfAxis.
     */
    struct SpherocylinderData {
        /** @brief Position of the mass centre of the spherocylinder. */
        const Vector<3> position;
        /** @brief The vector joining mass centre with one of the caps. */
        const Vector<3> halfAxis;
        /** @brief The norm of the above vector. */
        const double halfLength{};
        /** @brief Radius of the spherical cap (and also half-width of the spherocylinder). */
        const double radius{};
        /** @brief Spherocylinder's circumsphere radius. */
        const double circumsphereRadius{};

        SpherocylinderData(const Vector<3> &position, const Vector<3> &halfAxis, double radius);

        /**
         * @brief Returns Wolfram Mathematica representation of the spherocylinder calculated for position and
         * orientation of @a shape.
         */
        void toWolfram(std::ostream &out, const Shape &shape) const;

        /**
         * @brief Creates AbstractXCGeometry for the default-oriented spherocylinder (placed at [0, 0, 0]).
         */
        [[nodiscard]] std::shared_ptr<AbstractXCGeometry> createXCGeometry() const;

        [[nodiscard]] double getVolume() const;

        /**
         * @brief Returns the sphere's center calculated for position and orientation of @a shape.
         */
        [[nodiscard]] Vector<3> centreForShape(const Shape &shape) const;

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

private:
    std::vector<SpherocylinderData> spherocylinderData;
    std::optional<Vector<3>> primaryAxis;
    std::optional<Vector<3>> secondaryAxis;
    Vector<3> geometricOrigin;
    std::map<std::string, Vector<3>> namedPoints;
    double volume{};

    [[nodiscard]] double calculateVolume() const;

public:
    /**
     * @brief Constructs the polyspherocylinder with given parameters.
     * @details Apart from the named points passed in the @a customNamedPoints argument, 3 series of named point are
     * created automatically: `o[index]` (midpoint of spherocylinder), `b[index]` (center of the first spherical cap),
     * and `e[index]` (center of the second spherical cap), where `[index]` is the (0-based) index of the spherocylinder
     * in @a spherocylinderData vector.
     * @param spherocylinderData set of spherocylinders
     * @param primaryAxis the primary axis of the molecule
     * @param secondaryAxis the secondary axis of the polymer (should be orthogonal to the primary one)
     * @param geometricOrigin geometric origin of the molecule which can be different that the mass centre
     * @param volume volume of the shape. If `std::nullopt` is passed, the volume will be calculated automatically, but
     * only if the spherocylinders in @a spherocylinderData do not overlap. If they do overlap, the volume must be
     * computed manually by the caller and passed here
     * @param customNamedPoints optional map of name points, where the key is point's name and the value is its position
     */
    PolyspherocylinderShape(std::vector<SpherocylinderData> spherocylinderData, OptionalAxis primaryAxis,
                            OptionalAxis secondaryAxis, const Vector<3> &geometricOrigin = {0, 0, 0},
                            std::optional<double> volume = 0,
                            const std::map<std::string, Vector<3>> &customNamedPoints = {});

    [[nodiscard]] double getVolume() const /* override */ { return this->volume; }
    [[nodiscard]] Vector<3> getPrimaryAxis() const /* override */;
    [[nodiscard]] Vector<3> getSecondaryAxis() const /* override */;
    [[nodiscard]] Vector<3> getGeometricOrigin() const /* override */;
    [[nodiscard]] const std::map<std::string, Vector<3>> &getNamedPoints() const /* override */ {
        return this->namedPoints;
    }

    [[nodiscard]] const std::vector<SpherocylinderData> &getSpherocylinderData() const {
        return this->spherocylinderData;
    }

    /**
     * @brief Returns @a true is the constituent spherocylinders overlap, @a false otherwise.
     */
    [[nodiscard]] bool spherocylindersOverlap() const;

    void setGeometricOrigin(const Vector<3> &geometricOrigin_) { this->geometricOrigin = geometricOrigin_; }
    void addCustomNamedPoints(std::map<std::string, Vector<3>> customNamedPoints);

    friend bool operator==(const PolyspherocylinderShape &lhs, const PolyspherocylinderShape &rhs);
};


/**
 * @brief A class analogous to PolysphereTraits, but for hard spherocylinders.
 */
class PolyspherocylinderTraits
        : public ShapeTraits, public GenericShapeRegistry<PolyspherocylinderShape>, public Interaction
{
private:
    class WolframPrinter : public ShapePrinter {
    private:
        const PolyspherocylinderTraits &traits;

    public:
        explicit WolframPrinter(const PolyspherocylinderTraits &traits) : traits{traits} { }

        [[nodiscard]] std::string print(const Shape &shape) const override;
    };

    [[nodiscard]] std::shared_ptr<ShapePrinter> createObjPrinter(std::size_t subdivisions) const;

    std::shared_ptr<WolframPrinter> wolframPrinter;

    friend WolframPrinter;

public:
    /**
     * @brief The default number of sphere subdivisions when printing the shape (see XCPrinter::buildPolyhedron
     * @a subdivisions parameter)
     */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 3;

    /**
     * @brief Creates the class with  no initially registered species.
     */
    PolyspherocylinderTraits() : wolframPrinter{std::make_shared<WolframPrinter>(*this)} { }

    /**
     * @brief Creates the class with one initial species @a defaultSpecies named `A`, which is set as a default species
     * (setDefaultSpecies()).
     */
    explicit PolyspherocylinderTraits(const PolyspherocylinderShape &defaultSpecies);

    PolyspherocylinderTraits(const PolyspherocylinderTraits &) = delete;
    PolyspherocylinderTraits &operator=(const PolyspherocylinderTraits &) = delete;

    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }
    [[nodiscard]] bool hasWallPart() const override { return true; }
    [[nodiscard]] bool isConvex() const override { return false; }
    [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, const std::byte *data1,
                                      std::size_t idx1, const Vector<3> &pos2, const Matrix<3, 3> &orientation2,
                                      const std::byte *data2, std::size_t idx2,
                                      const BoundaryConditions &bc) const override;
    [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, const std::byte *data,
                                       std::size_t idx, const Vector<3> &wallOrigin,
                                       const Vector<3> &wallVector) const override;

    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres(const std::byte *data) const override;
    [[nodiscard]] double getRangeRadius([[maybe_unused]] const std::byte *data) const override;

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] const ShapeDataManager &getDataManager() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }

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
};


#endif //RAMPACK_POLYSPHEROCYLINDERTRAITS_H
