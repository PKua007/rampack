//
// Created by Piotr Kubala on 03/01/2023.
//

#ifndef RAMPACK_ARRANGEMENTMATCHER_H
#define RAMPACK_ARRANGEMENTMATCHER_H

#include "pyon/Matcher.h"
#include "core/Packing.h"


class ArrangementMatcher {
public:
    static pyon::matcher::MatcherAlternative create();
};


#endif //RAMPACK_ARRANGEMENTMATCHER_H
