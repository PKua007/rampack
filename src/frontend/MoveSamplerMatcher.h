//
// Created by Piotr Kubala on 05/01/2023.
//

#ifndef RAMPACK_MOVESAMPLERMATCHER_H
#define RAMPACK_MOVESAMPLERMATCHER_H

#include "pyon/Matcher.h"
#include "core/ShapeTraits.h"


class MoveSamplerMatcher {
public:
    static pyon::matcher::MatcherAlternative create(const ShapeTraits &traits);
};


#endif //RAMPACK_MOVESAMPLERMATCHER_H
