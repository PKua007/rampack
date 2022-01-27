//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SPHEROCYLINDERTRAITS_H
#define RAMPACK_SPHEROCYLINDERTRAITS_H

#include "core/ShapeTraits.h"

/**
 * @brief Hard spherocylinder.
 */
class SpherocylinderTraits : public ShapeTraits, public ShapePrinter, public Interaction {
private:
    double length{};    // distance between two spherical caps centres
    double radius{};    // radius of spherical caps

    [[nodiscard]] Vector<3> getCapCentre(short beginOrEnd, const Shape &shape) const;

public:
    /**
     * @brief Creates a spherocylinder spanned on x axis with a unit distance between cap centers and a unit radius.
     */
    SpherocylinderTraits() : length{1}, radius{1} { }

    /**
     * @brief Creates a spherocylinder spanned on x axis with @a lenght distance between cap centers and @a radius
     * radius.
     */
    SpherocylinderTraits(double length, double radius);

    [[nodiscard]] const Interaction &getInteraction() const override { return *this; }
    [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }
    [[nodiscard]] Vector<3> getPrimaryAxis(const Shape &shape) const override;
    [[nodiscard]] double getVolume() const override;

    [[nodiscard]] bool hasHardPart() const override { return true; }
    [[nodiscard]] bool hasSoftPart() const override { return false; }
    [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                      const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                      const BoundaryConditions &bc) const override;
    [[nodiscard]] double getRangeRadius() const override { return 2*this->radius + this->length; };

    [[nodiscard]] std::string toWolfram(const Shape &shape) const override;
};


#endif //RAMPACK_SPHEROCYLINDERTRAITS_H
