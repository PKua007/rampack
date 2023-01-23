//
// Created by Piotr Kubala on 06/01/2023.
//

#ifndef RAMPACK_BOXSCALERMATCHER_H
#define RAMPACK_BOXSCALERMATCHER_H

#include "pyon/Matcher.h"


class BoxScalerMatcher {
public:
    static pyon::matcher::MatcherAlternative create();
};


#endif //RAMPACK_BOXSCALERMATCHER_H
