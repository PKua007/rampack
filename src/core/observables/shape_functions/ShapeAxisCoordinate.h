//
// Created by Piotr Kubala on 23/03/2023.
//

#ifndef RAMPACK_SHAPEAXISCOORDINATE_H
#define RAMPACK_SHAPEAXISCOORDINATE_H

#include "core/observables/ShapeFunction.h"


/**
 * @brief Function returning a given coordinate value of a given shape axis
 */
class ShapeAxisCoordinate : public ShapeFunction {
private:
    ShapeGeometry::Axis axis{};
    std::size_t coord{};
    std::string name;

    [[nodiscard]] std::string constructName() const;

public:
    /**
     * @brief Constructs the function, which returns @a coord coordinate (0 - x, 1 - y, 2 - z) of @a axis shape axis.
     */
    ShapeAxisCoordinate(ShapeGeometry::Axis axis, std::size_t coord);

    [[nodiscard]] double calculate(const Shape &shape, const ShapeTraits &traits) const override;
    [[nodiscard]] std::string getName() const override { return this->name; }
};


#endif //RAMPACK_SHAPEAXISCOORDINATE_H
