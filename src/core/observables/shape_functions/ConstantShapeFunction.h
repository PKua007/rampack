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

    [[nodiscard]] double calculate([[maybe_unused]] const Shape &shape,
                                   [[maybe_unused]] const ShapeTraits &traits) const override
    {
        return this->value;
    }

    [[nodiscard]] std::string getName() const override { return "const"; }
};


#endif //RAMPACK_CONSTANTSHAPEFUNCTION_H
