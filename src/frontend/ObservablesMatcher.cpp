//
// Created by Piotr Kubala on 30/12/2022.
//

#include "ObservablesMatcher.h"
#include "utils/OMPMacros.h"

pyon::matcher::MatcherArray ObservableMatcher::createObservablesMatcher(std::size_t maxThreads) {
    if (maxThreads == 0)
        maxThreads = OMP_MAXTHREADS;

    return {};
}

pyon::matcher::MatcherArray ObservableMatcher::createBullObservablesMatcher(std::size_t maxThreads) {
    if (maxThreads == 0)
        maxThreads = OMP_MAXTHREADS;

    return {};
}
