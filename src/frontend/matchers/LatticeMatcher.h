//
// Created by Piotr Kubala on 03/01/2023.
//

#ifndef RAMPACK_LATTICEMATCHER_H
#define RAMPACK_LATTICEMATCHER_H

#include "pyon/Matcher.h"
#include "core/lattice/LatticeTransformer.h"


class LatticeMatcher {
public:
    static pyon::matcher::MatcherAlternative create();

    static pyon::matcher::MatcherAlternative createIrregularLatticeTransformer();
    static std::shared_ptr<LatticeTransformer> matchIrregularLatticeTransformer(const std::string &expression);
};


#endif //RAMPACK_LATTICEMATCHER_H
