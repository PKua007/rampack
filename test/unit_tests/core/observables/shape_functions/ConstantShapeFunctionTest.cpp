//
// Created by Piotr Kubala on 24/03/2023.
//

#include <catch2/catch.hpp>

#include "core/observables/shape_functions/ConstantShapeFunction.h"
#include "core/shapes/SphereTraits.h"


TEST_CASE("ConstantShapeFunction") {
    SphereTraits traits;
    Shape shape({0, 1, 2}, Matrix<3, 3>::rotation(0.1, 0.2, 0.3));

    SECTION("default") {
        ConstantShapeFunction constantShapeFunction;

        constantShapeFunction.calculate(shape, traits);

        CHECK(constantShapeFunction.getValues() == std::vector<double>{1});
        CHECK(constantShapeFunction.getNames() == std::vector<std::string>{"const"});
    }

    SECTION("given") {
        ConstantShapeFunction constantShapeFunction(3);

        constantShapeFunction.calculate(shape, traits);

        CHECK(constantShapeFunction.getValues() == std::vector<double>{3});
    }
}