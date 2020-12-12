//
// Created by Piotr Kubala on 12/12/2020.
//

#include "ShapeFactory.h"
#include "utils/Assertions.h"
#include "Sphere.h"

std::unique_ptr<Shape> ShapeFactory::createShape(const std::string &shapeName, const std::string &shapeAttributes) {
    if (shapeName == "Sphere")
        return std::make_unique<Sphere>(std::stoul(shapeAttributes));
    else
        throw ValidationException("Unknown particle name: " + shapeName);
}
