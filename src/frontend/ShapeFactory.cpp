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
                                                          const std::string &interaction)
{
    std::istringstream shapeAttrStream(shapeAttributes);
    std::istringstream interactionAttrStream(interaction);
    std::string interactionName;
    interactionAttrStream >> interactionName;
    if (shapeName == "Sphere") {
        double r;
        shapeAttrStream >> r;
        ValidateMsg(shapeAttrStream, "Malformed Sphere attributes; expected: [radius]");
        Validate(r > 0);
        if (interactionName.empty() || interactionName == "hard") {
            return std::make_unique<SphereTraits>(r, SphereTraits::HardInteraction{});
        } else if (interactionName == "lj") {
            double epsilon, sigma;
            interactionAttrStream >> epsilon >> sigma;
            ValidateMsg(interactionAttrStream, "Malformed Lennard Jones attributes. Usage: lj [epsilon] [sigma]");
            Validate(epsilon > 0);
            Validate(sigma > 0);
            return std::make_unique<SphereTraits>(r, SphereTraits::LennardJonesInteraction(epsilon, sigma));
        } else {
            throw ValidationException("Sphere support interactions: hard, lj (Lennard Jones)");
        }
    } else if (shapeName == "Spherocylinder") {
        double r, length;
        shapeAttrStream >> length >> r;
        ValidateMsg(shapeAttrStream, "Malformed Spherocylinder attributes; expected: [length] [radius]");
        Validate(r > 0);
        Validate(length >= 0);
        ValidateMsg(interactionName.empty(), "Spherocylinder supports only hard interactions");
        return std::make_unique<SpherocylinderTraits>(length, r);
    } else {
        throw ValidationException("Unknown particle name: " + shapeName);
    }
}
