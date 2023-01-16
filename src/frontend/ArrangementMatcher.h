//
// Created by Piotr Kubala on 03/01/2023.
//

#ifndef RAMPACK_ARRANGEMENTMATCHER_H
#define RAMPACK_ARRANGEMENTMATCHER_H

#include "pyon/Matcher.h"
#include "core/Packing.h"
#include "core/ShapeTraits.h"


class ArrangementMatcher {
public:
    static pyon::matcher::MatcherAlternative create(const ShapeTraits &traits);
};


#endif //RAMPACK_ARRANGEMENTMATCHER_H
