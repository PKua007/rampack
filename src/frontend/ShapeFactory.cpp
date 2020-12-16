//
// Created by Piotr Kubala on 12/12/2020.
//

#include <sstream>

#include "ShapeFactory.h"
#include "utils/Assertions.h"
#include "core/shapes/Sphere.h"
#include "core/shapes/Spherocylinder.h"

std::unique_ptr<Shape> ShapeFactory::createShape(const std::string &shapeName, const std::string &shapeAttributes) {
    std::istringstream attributesStream(shapeAttributes);
    if (shapeName == "Sphere") {
        double r;
        attributesStream >> r;
        ValidateMsg(attributesStream, "Malformed Sphere attributes; expected: [radius]");
        Validate(r > 0);
        return std::make_unique<Sphere>(r);
    } else if (shapeName == "Spherocylinder") {
        double r, length;
        attributesStream >> length >> r;
        ValidateMsg(attributesStream, "Malformed Spherocylinder attributes; expected: [length] [radius]");
        Validate(r > 0);
        Validate(length >= 0);
        return std::make_unique<Spherocylinder>(length, r);
    } else {
        throw ValidationException("Unknown particle name: " + shapeName);
    }
}
