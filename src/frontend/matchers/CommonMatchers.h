//
// Created by Piotr Kubala on 12/03/2024.
//

#ifndef RAMPACK_COMMONMATCHERS_H
#define RAMPACK_COMMONMATCHERS_H

#include "pyon/Matcher.h"


class CommonMatchers {
public:
    static const pyon::matcher::MatcherString &createSymbol();
    static const pyon::matcher::MatcherDictionary &createShapeSpeciesMap();
};


#endif //RAMPACK_COMMONMATCHERS_H
