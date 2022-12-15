//
// Created by pkua on 10.12.22.
//

#ifndef RAMPACK_MATCHER_H
#define RAMPACK_MATCHER_H

#include "MatcherNumeral.h"
#include "MatcherBoolean.h"
#include "MatcherNone.h"
#include "MatcherString.h"
#include "MatcherArray.h"
#include "MatcherDictionary.h"
#include "MatcherDataclass.h"
#include "MatcherAlternative.h"


namespace pyon::matcher {
    template<typename ConcreteMatcher1, typename ConcreteMatcher2>
    MatcherAlternative operator|(const ConcreteMatcher1 &matcher1, const ConcreteMatcher2 &matcher2) {
        static_assert(std::is_base_of_v<MatcherBase, ConcreteMatcher1>,
                      "ConcreteMatcher1 template parameter is not a matcher derived from MatcherBase");
        static_assert(std::is_base_of_v<MatcherBase, ConcreteMatcher2>,
                      "ConcreteMatcher1 template parameter is not a matcher derived from MatcherBase");

        return (MatcherAlternative{} |= matcher1) |= matcher2;
    }
}

#endif //RAMPACK_MATCHER_H
