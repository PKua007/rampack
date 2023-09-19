//
// Created by Piotr Kubala on 19/09/2023.
//

#ifndef RAMPACK_SHAPEAXIS_H
#define RAMPACK_SHAPEAXIS_H

#include "core/observables/ShapeFunction.h"

/**
 * @brief Function returning a given shape axis.
 */
class ShapeAxis : public ShapeFunction {
private:
    std::vector<double> values;
    ShapeGeometry::Axis axis{};

public:
    /**
     * @brief Construct the class returning the axis given by @a axis.
     */
    explicit ShapeAxis(ShapeGeometry::Axis axis) : values(3), axis{axis} { }

    void calculate(const Shape &shape, const ShapeTraits &traits) override;

    /**
     * @brief Returns `pa`, `sa` or `aa`, standing for, respectively, primary, secondary and auxiliary axis, depending
     * on what was has been chosen in the constructor.
     */
    [[nodiscard]] std::string getPrimaryName() const override;

    /**
     * @brief Returns `x`, `y` and `z` as a function components.
     */
    [[nodiscard]] std::vector<std::string> getNames() const override { return {"x", "y", "z"}; }

    /**
     * @brief Returns subsequents x, y and z components of the shape axis.
     */
    [[nodiscard]] std::vector<double> getValues() const override { return this->values; };
};


#endif //RAMPACK_SHAPEAXIS_H
