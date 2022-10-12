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
class SphereTraits : public ShapeTraits, public ShapePrinter, public ShapeGeometry {
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
        [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                          const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                          const BoundaryConditions &bc) const override;
        [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, std::size_t idx,
                                           const Vector<3> &wallOrigin, const Vector<3> &wallVector) const override;
        [[nodiscard]] double getRangeRadius() const override { return 2 * this->radius; }
    };

    double radius{};
    std::unique_ptr<Interaction> interaction{};

public:
    /**
     * @brief Creates a hard sphere.
     */
    explicit SphereTraits(double radius = 1);

    /**
     * @brief Creates a sphere interacting via @a centralInteraction soft potential.
     */
    SphereTraits(double radius, std::unique_ptr<CentralInteraction> centralInteraction);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this->interaction; };
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }
    [[nodiscard]] const ShapeGeometry &getGeometry() const override { return *this; }
    [[nodiscard]] double getVolume() const override;

    [[nodiscard]] std::string toWolfram(const Shape &shape) const override;

    [[nodiscard]] double getRadius() const { return this->radius; }
};


#endif //RAMPACK_SPHERETRAITS_H
