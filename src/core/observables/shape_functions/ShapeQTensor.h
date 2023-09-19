//
// Created by Piotr Kubala on 19/09/2023.
//

#ifndef RAMPACK_SHAPEQTENSOR_H
#define RAMPACK_SHAPEQTENSOR_H

#include "core/observables/ShapeFunction.h"


class ShapeQTensor : public ShapeFunction {
private:
    std::vector<double> values;
    ShapeGeometry::Axis axis{};

public:
    explicit ShapeQTensor(ShapeGeometry::Axis axis) : values(6), axis{axis} { }

    void calculate(const Shape &shape, const ShapeTraits &traits) override;

    [[nodiscard]] std::string getPrimaryName() const override;

    [[nodiscard]] std::vector<std::string> getNames() const override {
        return {"c11", "c12", "c13", "c22", "c23", "c33"};
    }

    [[nodiscard]] std::vector<double> getValues() const override { return this->values; };
};


#endif //RAMPACK_SHAPEQTENSOR_H
