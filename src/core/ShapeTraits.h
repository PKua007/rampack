//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SHAPETRAITS_H
#define RAMPACK_SHAPETRAITS_H

#include "Shape.h"
#include "Interaction.h"
#include "ShapePrinter.h"
#include "ShapeGeometry.h"
#include "geometry/Vector.h"

/**
 * @brief An interface describing a concrete shape.
 */
class ShapeTraits {
public:
    virtual ~ShapeTraits() = default;

    /**
     * @brief Returns the Interaction object describing the interaction of the shape.
     */
    [[nodiscard]] virtual const Interaction &getInteraction() const = 0;

    /**
     * @brief Returns the ShapePrinter object describing the geometry of the shape.
     */
    [[nodiscard]] virtual const ShapeGeometry &getGeometry() const = 0;

    /**
     * @brief Returns the ShapePrinter object responsible for shape printing in supported format.
     */
    [[nodiscard]] virtual const ShapePrinter &getPrinter(const std::string &format) const = 0;
};

#endif //RAMPACK_SHAPETRAITS_H
