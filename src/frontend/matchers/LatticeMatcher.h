//
// Created by Piotr Kubala on 03/01/2023.
//

#ifndef RAMPACK_LATTICEMATCHER_H
#define RAMPACK_LATTICEMATCHER_H

#include "pyon/Matcher.h"


class LatticeMatcher {
public:
    static pyon::matcher::MatcherAlternative create();
};


#endif //RAMPACK_LATTICEMATCHER_H
