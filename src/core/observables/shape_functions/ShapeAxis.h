//
// Created by Piotr Kubala on 19/09/2023.
//

#ifndef RAMPACK_SHAPEAXIS_H
#define RAMPACK_SHAPEAXIS_H

#include "core/observables/ShapeFunction.h"


class ShapeAxis : public ShapeFunction {
private:
    std::vector<double> values;
    ShapeGeometry::Axis axis{};

public:
    explicit ShapeAxis(ShapeGeometry::Axis axis) : values(3), axis{axis} { }

    void calculate(const Shape &shape, const ShapeTraits &traits) override;
    [[nodiscard]] std::string getPrimaryName() const override;
    [[nodiscard]] std::vector<std::string> getNames() const override { return {"x", "y", "z"}; }
    [[nodiscard]] std::vector<double> getValues() const override { return this->values; };
};


#endif //RAMPACK_SHAPEAXIS_H
