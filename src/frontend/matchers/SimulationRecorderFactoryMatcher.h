//
// Created by Piotr Kubala on 23/01/2023.
//

#ifndef RAMPACK_SIMULATIONRECORDERFACTORYMATCHER_H
#define RAMPACK_SIMULATIONRECORDERFACTORYMATCHER_H

#include <memory>

#include "pyon/Matcher.h"
#include "frontend/SimulationRecorderFactory.h"


class SimulationRecorderFactoryMatcher {
public:
    static pyon::matcher::MatcherAlternative create();
    static std::shared_ptr<SimulationRecorderFactory> match(const std::string &expression);
};


#endif //RAMPACK_SIMULATIONRECORDERFACTORYMATCHER_H
