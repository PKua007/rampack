//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SPHEROCYLINDERTRAITS_H
#define RAMPACK_SPHEROCYLINDERTRAITS_H

#include "core/ShapeTraits.h"

/**
 * @brief Hard spherocylinder spanned on Z axis.
 * @details Primary axis is naturally Z axis. Mass centre coincides with geometric origin. The class, apart from
 * standard named points (see ShapeGeometry::getNamedPoint()) defines points "beg" and "end" representing, respectively,
 * beginning cap center and end cap center of the spherocylinder.
 */
class SpherocylinderTraits : public ShapeTraits, public Interaction, public ShapeGeometry {
private:
    class WolframPrinter : public ShapePrinter {
    private:
        const SpherocylinderTraits &traits;

    public:
        explicit WolframPrinter(const SpherocylinderTraits &traits) : traits{traits} { }
        [[nodiscard]] std::string print(const Shape &shape) const override;
    };

    double length{};    // distance between two spherical caps centres
    double radius{};    // radius of spherical caps
    std::shared_ptr<WolframPrinter> wolframPrinter;

    static std::unique_ptr<ShapePrinter> createObjPrinter(double length, double radius, std::size_t subdivisions);

    [[nodiscard]] Vector<3> getCapCentre(short beginOrEnd, const Shape &shape) const;

    friend WolframPrinter;

public:
    /** @brief The default number of sphere subdivisions when printing the shape (see XCPrinter::XCPrinter
     * @a subdivision parameter) */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 4;

    /**
     * @brief Creates a spherocylinder spanned on x axis with a unit distance between cap centers and a unit radius.
     */
    SpherocylinderTraits() : SpherocylinderTraits(1, 1) { }

    /**
     * @brief Creates a spherocylinder spanned on x axis with @a length distance between cap centers and @a radius
     * radius.
     */
    SpherocylinderTraits(double length, double radius);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }

    /**
     * @brief Returns ShapePrinter for a given @a format.
     * @details The following formats are supported:
     * <ol>
     *     <li> `wolfram` - Wolfram Mathematica shape
     *     <li> `obj` - Wavefront OBJ triangle mesh (it accepts @a mesh_divisions parameter, default: 4)
     * </ol>
     */
    [[nodiscard]] std::shared_ptr<const ShapePrinter>
    getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const override;

    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const override;
    [[nodiscard]] double getVolume() const override;

    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasWallPart() const override { return true; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }
    [[nodiscard]] bool isConvex() const override { return true; }
    [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                      const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                      const BoundaryConditions &bc) const override;

    [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, std::size_t idx,
                                       const Vector<3> &wallOrigin, const Vector<3> &wallVector) const override;

    [[nodiscard]] double getRangeRadius() const override { return 2*this->radius + this->length; };
};


#endif //RAMPACK_SPHEROCYLINDERTRAITS_H
