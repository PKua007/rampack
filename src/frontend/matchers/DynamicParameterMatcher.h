//
// Created by Piotr Kubala on 07/01/2023.
//

#ifndef RAMPACK_DYNAMICPARAMETERMATCHER_H
#define RAMPACK_DYNAMICPARAMETERMATCHER_H

#include "pyon/Matcher.h"
#include "core/DynamicParameter.h"


class DynamicParameterMatcher {
public:
    static pyon::matcher::MatcherAlternative create();
};


#endif //RAMPACK_DYNAMICPARAMETERMATCHER_H
