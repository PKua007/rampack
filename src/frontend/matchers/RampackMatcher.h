//
// Created by Piotr Kubala on 20/01/2023.
//

#ifndef RAMPACK_RAMPACKMATCHER_H
#define RAMPACK_RAMPACKMATCHER_H

#include "pyon/Matcher.h"
#include "frontend/RampackParameters.h"


class RampackMatcher {
public:
    static pyon::matcher::MatcherDataclass create();
    static RampackParameters match(const std::string &expression);
};


#endif //RAMPACK_RAMPACKMATCHER_H
