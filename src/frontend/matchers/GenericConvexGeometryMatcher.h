//
// Created by Piotr Kubala on 01/06/2023.
//

#ifndef RAMPACK_GENERICCONVEXGEOMETRYMATCHER_H
#define RAMPACK_GENERICCONVEXGEOMETRYMATCHER_H

#include <functional>

#include "pyon/Matcher.h"
#include "geometry/xenocollide/XCBodyBuilder.h"


using XCBodyBuilderScript = std::function<void(XCBodyBuilder&)>;

class GenericConvexGeometryMatcher {
private:
    static pyon::matcher::RecursiveMatcher shapeRecursion;
    static pyon::matcher::MatcherAlternative create();

public:
    static pyon::matcher::MatcherAlternative script;
    static XCBodyBuilderScript match(const std::string &expression);
};


#endif //RAMPACK_GENERICCONVEXGEOMETRYMATCHER_H
