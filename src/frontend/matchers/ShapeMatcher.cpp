//
// Created by Piotr Kubala on 16/12/2022.
//

#include "ShapeMatcher.h"

#include "shapes/PolysphereMatcher.h"
#include "shapes/PolyspherocylinderMatcher.h"
#include "shapes/XenoCollideMatcher.h"


std::shared_ptr<ShapeTraits> ShapeMatcher::match(const std::string &expression) {
    pyon::matcher::Any shapeTraits;
    auto shapeAST = pyon::Parser::parse(expression);
    auto shapeMatcher = ShapeMatcher::create();
    auto matchReport = shapeMatcher.match(shapeAST, shapeTraits);
    if (!matchReport)
        throw ValidationException(matchReport.getReason());

    return shapeTraits.as<std::shared_ptr<ShapeTraits>>();
}

pyon::matcher::MatcherAlternative ShapeMatcher::create() {
    return PolysphereMatcher::create()
        | PolyspherocylinderMatcher::create()
        | XenoCollideMatcher::create();
}
