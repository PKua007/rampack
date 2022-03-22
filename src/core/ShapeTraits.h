//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SHAPETRAITS_H
#define RAMPACK_SHAPETRAITS_H

#include "Shape.h"
#include "Interaction.h"
#include "ShapePrinter.h"
#include "geometry/Vector.h"

/**
 * @brief An interface describing a concrete shape.
 */
class ShapeTraits {
public:
    virtual ~ShapeTraits() = default;

    /**
     * @brief Returns the Interaction object describing the interaction of the shape (hard of soft).
     */
    [[nodiscard]] virtual const Interaction &getInteraction() const = 0;

    /**
     * @brief Returns the volume of the shape.
     */
    [[nodiscard]] virtual double getVolume() const = 0;

    /**
     * @brief Returns the primary (long) molecular axis for a given @a shape.
     */
    [[nodiscard]] virtual Vector<3> getPrimaryAxis([[maybe_unused]] const Shape &shape) const {
        throw std::runtime_error("ShapeTraits::getPrimaryAxis : unsupported");
    }

    /**
     * @brief Returns the secondary (polarization) molecular axis for a given @a shape.
     */
    [[nodiscard]] virtual Vector<3> getSecondaryAxis([[maybe_unused]] const Shape &shape) const {
        throw std::runtime_error("ShapeTraits::getSecondaryAxis : unsupported");
    }

    /**
     * @brief Returns the ShapePrinter object responsible for shape printing in supported format.
     */
    [[nodiscard]] virtual const ShapePrinter &getPrinter() const = 0;
};

#endif //RAMPACK_SHAPETRAITS_H
