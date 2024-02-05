//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SPHERETRAITS_H
#define RAMPACK_SPHERETRAITS_H

#include <variant>

#include "core/ShapeTraits.h"
#include "core/interactions/CentralInteraction.h"

/**
 * @brief Spherical molecules with hard of soft interactions.
 */
class SphereTraits : public ShapeTraits, public ShapeGeometry {
private:
    class HardInteraction : public Interaction {
    private:
        double radius{};

    public:
        explicit HardInteraction(double radius) : radius(radius) { }

        [[nodiscard]] bool hasHardPart() const override { return true; }
        [[nodiscard]] bool hasSoftPart() const override { return false; }
        [[nodiscard]] bool hasWallPart() const override { return true; }
        [[nodiscard]] bool isConvex() const override { return true; }
        [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                          const std::byte *data1, std::size_t idx1, const Vector<3> &pos2,
                                          const Matrix<3, 3> &orientation2, const std::byte *data2, std::size_t idx2,
                                          const BoundaryConditions &bc) const override;
        [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, const std::byte *data,
                                           std::size_t idx, const Vector<3> &wallOrigin,
                                           const Vector<3> &wallVector) const override;
        [[nodiscard]] double getRangeRadius([[maybe_unused]] const std::byte *data) const override {
            return 2 * this->radius;
        }
    };

    class WolframPrinter : public ShapePrinter {
    private:
        double radius{};

    public:
        explicit WolframPrinter(double radius) : radius{radius} { }
        [[nodiscard]] std::string print(const Shape &shape) const override;
    };

    static std::shared_ptr<ShapePrinter> createObjPrinter(double radius, std::size_t subdivisions);

    double radius{};
    std::shared_ptr<Interaction> interaction{};
    std::shared_ptr<WolframPrinter> wolframPrinter;

public:
    /** @brief The default number of sphere subdivisions when printing the shape (see XCPrinter::XCPrinter
     * @a subdivision parameter) */
    static constexpr std::size_t DEFAULT_MESH_SUBDIVISIONS = 3;

    /**
     * @brief Creates a hard sphere.
     */
    explicit SphereTraits(double radius = 1);

    /**
     * @brief Creates a sphere interacting via @a centralInteraction soft potential.
     */
    SphereTraits(double radius, std::shared_ptr<CentralInteraction> centralInteraction);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this->interaction; };

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

    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }
    [[nodiscard]] double getVolume(const Shape &shape) const override;

    [[nodiscard]] double getRadius() const { return this->radius; }
};


#endif //RAMPACK_SPHERETRAITS_H
