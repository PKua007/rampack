//
// Created by Piotr Kubala on 30/12/2022.
//

#ifndef RAMPACK_OBSERVABLESMATCHER_H
#define RAMPACK_OBSERVABLESMATCHER_H

#include <memory>

#include "pyon/Matcher.h"
#include "core/Observable.h"


class ObservablesMatcher {
public:
    struct ObservableData {
        std::size_t scope{};
        std::shared_ptr<Observable> observable;
    };

    static pyon::matcher::MatcherArray createObservablesMatcher(std::size_t maxThreads = 1);
    static pyon::matcher::MatcherArray createBulkObservablesMatcher(std::size_t maxThreads = 1);
};


#endif //RAMPACK_OBSERVABLESMATCHER_H
