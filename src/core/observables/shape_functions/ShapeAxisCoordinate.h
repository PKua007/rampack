//
// Created by Piotr Kubala on 23/03/2023.
//

#ifndef RAMPACK_SHAPEAXISCOORDINATE_H
#define RAMPACK_SHAPEAXISCOORDINATE_H

#include "core/observables/ShapeFunction.h"


/**
 * @brief Function returning a given coordinate value of a given shape axis.
 * @details Both the primary name and a singular component name are `[axis]_[comp]`, where `[axis]` is `pa`, `sa` or
 * `aa`, referring to, respectively` primary, secondary and auxiliary axis (see ShapeGeometry) and `[comp]` is `x`, `y`
 * or `z`.
 */
class ShapeAxisCoordinate : public ShapeFunction {
private:
    ShapeGeometry::Axis axis{};
    std::size_t coord{};
    std::string name;
    double value{};

    [[nodiscard]] std::string constructName() const;

public:
    /**
     * @brief Constructs the function, which returns @a coord coordinate (0 - x, 1 - y, 2 - z) of @a axis shape axis.
     */
    ShapeAxisCoordinate(ShapeGeometry::Axis axis, std::size_t coord);

    void calculate(const Shape &shape, const ShapeTraits &traits) override;

    [[nodiscard]] std::string getPrimaryName() const override { return this->name; }
    [[nodiscard]] std::vector<std::string> getNames() const override { return {this->name}; }
    [[nodiscard]] std::vector<double> getValues() const override { return {this->value}; }
};


#endif //RAMPACK_SHAPEAXISCOORDINATE_H
