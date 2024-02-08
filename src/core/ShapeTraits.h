//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SHAPETRAITS_H
#define RAMPACK_SHAPETRAITS_H

#include <memory>
#include <map>

#include "Shape.h"
#include "Interaction.h"
#include "ShapePrinter.h"
#include "ShapeGeometry.h"
#include "ShapeDataManager.h"
#include "geometry/Vector.h"


/**
 * @brief An interface describing a concrete shape.
 */
class ShapeTraits {
public:
    virtual ~ShapeTraits() = default;

    [[nodiscard]] virtual const ShapeDataManager &getDataManager() const = 0;

    /**
     * @brief Returns the Interaction object describing the interaction of the shape.
     */
    [[nodiscard]] virtual const Interaction &getInteraction() const = 0;

    /**
     * @brief Returns the ShapePrinter object describing the geometry of the shape.
     */
    [[nodiscard]] virtual const ShapeGeometry &getGeometry() const = 0;

    /**
     * @brief Returns the ShapePrinter object responsible for shape printing in a given @a format.
     * @param format format of the printer ("wolfram", "obj", etc.)
     * @param params format- and shape-specific parameters (for example "mesh_subdivisions")
     * @throws NoSuchShapePrinterException when @a format doesn't specify existing ShapePrinter type
     */
    [[nodiscard]] virtual std::shared_ptr<const ShapePrinter>
    getPrinter(const std::string &format, const std::map<std::string, std::string> &params) const = 0;
};

#endif //RAMPACK_SHAPETRAITS_H
