//
// Created by Piotr Kubala on 03/01/2023.
//

#ifndef RAMPACK_LATTICEMATCHER_H
#define RAMPACK_LATTICEMATCHER_H

#include "pyon/Matcher.h"
#include "core/ShapeTraits.h"


class LatticeMatcher {
public:
    static pyon::matcher::MatcherAlternative create(const ShapeTraits &traits);
};


#endif //RAMPACK_LATTICEMATCHER_H
