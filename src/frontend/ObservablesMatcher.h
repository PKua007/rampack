//
// Created by Piotr Kubala on 30/12/2022.
//

#ifndef RAMPACK_OBSERVABLESMATCHER_H
#define RAMPACK_OBSERVABLESMATCHER_H

#include "pyon/Matcher.h"

class ObservableMatcher {
public:
    static pyon::matcher::MatcherArray createObservablesMatcher(std::size_t maxThreads = 1);
    static pyon::matcher::MatcherArray createBulkObservablesMatcher(std::size_t maxThreads = 1);
};


#endif //RAMPACK_OBSERVABLESMATCHER_H
