//
// Created by Piotr Kubala on 23/01/2023.
//

#ifndef RAMPACK_SIMULATIONRECORDERFACTORYMATCHER_H
#define RAMPACK_SIMULATIONRECORDERFACTORYMATCHER_H

#include "pyon/Matcher.h"


class SimulationRecorderFactoryMatcher {
public:
    static pyon::matcher::MatcherAlternative create();
};


#endif //RAMPACK_SIMULATIONRECORDERFACTORYMATCHER_H
