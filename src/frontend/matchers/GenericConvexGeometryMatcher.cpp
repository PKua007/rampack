//
// Created by Piotr Kubala on 01/06/2023.
//

#include "GenericConvexGeometryMatcher.h"


using namespace pyon::matcher;

namespace {
    MatcherAlternative create_script(const RecursiveMatcher &shapeRecursion);
    MatcherDataclass create_point();
    MatcherDataclass create_segment();
    MatcherDataclass create_rectangle();
    MatcherDataclass create_cuboid();
    MatcherDataclass create_disk();
    MatcherDataclass create_sphere();
    MatcherDataclass create_ellipse();
    MatcherDataclass create_ellipsoid();
    MatcherDataclass create_saucer();
    MatcherDataclass create_football();
    MatcherDataclass create_sum(const RecursiveMatcher &shapeRecursion);
    MatcherDataclass create_diff(const RecursiveMatcher &shapeRecursion);
    MatcherDataclass create_wrap(const RecursiveMatcher &shapeRecursion);


    MatcherDataclass create_point() {
        return MatcherDataclass("point");
    }

    MatcherDataclass create_segment() {
        return MatcherDataclass("segment");
    }

    MatcherDataclass create_rectangle() {
        return MatcherDataclass("rectangle");
    }

    MatcherDataclass create_cuboid() {
        return MatcherDataclass("cuboid");
    }

    MatcherDataclass create_disk() {
        return MatcherDataclass("disk");
    }

    MatcherDataclass create_sphere() {
        return MatcherDataclass("sphere");
    }

    MatcherDataclass create_ellipse() {
        return MatcherDataclass("ellipse");
    }

    MatcherDataclass create_ellipsoid() {
        return MatcherDataclass("ellipsoid");
    }

    MatcherDataclass create_saucer() {
        return MatcherDataclass("saucer");
    }

    MatcherDataclass create_football() {
        return MatcherDataclass("football");
    }

    MatcherDataclass create_sum(const RecursiveMatcher &shapeRecursion) {
        return MatcherDataclass("sum");
    }

    MatcherDataclass create_diff(const RecursiveMatcher &shapeRecursion) {
        return MatcherDataclass("diff");
    }

    MatcherDataclass create_wrap(const RecursiveMatcher &shapeRecursion) {
        return MatcherDataclass("wrap");
    }
    
    MatcherAlternative create_script(const RecursiveMatcher &shapeRecursion) {
        return create_point()
            | create_segment()
            | create_rectangle()
            | create_cuboid()
            | create_disk()
            | create_sphere()
            | create_ellipse()
            | create_ellipsoid()
            | create_saucer()
            | create_football()
            | create_sum(shapeRecursion)
            | create_diff(shapeRecursion)
            | create_wrap(shapeRecursion);
    }
}

RecursiveMatcher GenericConvexGeometryMatcher::shapeRecursion;
MatcherAlternative GenericConvexGeometryMatcher::script = GenericConvexGeometryMatcher::create();

MatcherAlternative GenericConvexGeometryMatcher::create() {
    auto matcher = create_script(GenericConvexGeometryMatcher::shapeRecursion);
    GenericConvexGeometryMatcher::shapeRecursion.attach(matcher);
    return matcher;
}

XCBodyBuilderScript GenericConvexGeometryMatcher::match(const std::string &expression) {
    auto scriptAST = pyon::Parser::parse(expression);
    Any writer;
    auto matchReport = GenericConvexGeometryMatcher::script.match(scriptAST, writer);
    if (!matchReport)
        throw ValidationException(matchReport.getReason());

    return writer.as<XCBodyBuilderScript>();
}
