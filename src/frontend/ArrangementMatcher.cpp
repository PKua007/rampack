//
// Created by Piotr Kubala on 03/01/2023.
//

#include "ArrangementMatcher.h"
#include "LatticeMatcher.h"

using namespace pyon::matcher;


namespace {
    MatcherDataclass create_presimulated() {
        return MatcherDataclass("presimulated");
    }
}


MatcherAlternative ArrangementMatcher::create() {
    return create_presimulated() | LatticeMatcher::create();
}
