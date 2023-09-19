//
// Created by Piotr Kubala on 23/03/2023.
//

#ifndef RAMPACK_CONSTANTSHAPEFUNCTION_H
#define RAMPACK_CONSTANTSHAPEFUNCTION_H

#include "core/observables/ShapeFunction.h"


/**
 * @brief A constant ShapeFunction.
 */
class ConstantShapeFunction : public ShapeFunction {
private:
    double value{};

public:
    explicit ConstantShapeFunction(double value = 1) : value{value} { }

    void calculate([[maybe_unused]] const Shape &shape, [[maybe_unused]] const ShapeTraits &traits) override { }

    [[nodiscard]] std::string getPrimaryName() const override { return "const"; }
    [[nodiscard]] std::vector<std::string> getNames() const override { return {"const"}; }
    [[nodiscard]] std::vector<double> getValues() const override { return {this->value}; }
};


#endif //RAMPACK_CONSTANTSHAPEFUNCTION_H
