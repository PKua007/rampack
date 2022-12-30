//
// Created by Piotr Kubala on 25/03/2021.
//

#ifndef RAMPACK_OBSERVABLESCOLLECTORFACTORY_H
#define RAMPACK_OBSERVABLESCOLLECTORFACTORY_H

#include <memory>
#include <string>
#include <vector>

#include "core/ObservablesCollector.h"
#include "utils/Version.h"


namespace legacy {
    class ObservablesCollectorFactory {
    public:
        static std::unique_ptr<ObservablesCollector> create(const std::vector<std::string> &observables,
                                                            const std::vector<std::string> &bulkObservables,
                                                            std::size_t maxThreads = 1,
                                                            const Version &version = CURRENT_VERSION);
    };
}


#endif //RAMPACK_OBSERVABLESCOLLECTORFACTORY_H
