//
// Created by Piotr Kubala on 12/12/2020.
//

#include <sstream>

#include "ShapeFactory.h"
#include "utils/Assertions.h"
#include "core/shapes/SphereTraits.h"
#include "core/shapes/SpherocylinderTraits.h"

std::unique_ptr<ShapeTraits> ShapeFactory::shapeTraitsFor(const std::string &shapeName,
                                                          const std::string &shapeAttributes,
                                                          [[maybe_unused]] const std::string &interactionName,
                                                          [[maybe_unused]] const std::string &interactionAttributes)
{
    std::istringstream attributesStream(shapeAttributes);
    if (shapeName == "Sphere") {
        double r;
        attributesStream >> r;
        ValidateMsg(attributesStream, "Malformed Sphere attributes; expected: [radius]");
        Validate(r > 0);
        return std::make_unique<SphereTraits>(r);
    } else if (shapeName == "Spherocylinder") {
        double r, length;
        attributesStream >> length >> r;
        ValidateMsg(attributesStream, "Malformed Spherocylinder attributes; expected: [length] [radius]");
        Validate(r > 0);
        Validate(length >= 0);
        return std::make_unique<SpherocylinderTraits>(length, r);
    } else {
        throw ValidationException("Unknown particle name: " + shapeName);
    }
}
