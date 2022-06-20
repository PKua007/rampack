//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_PARAMETERUPDATERFACTORY_H
#define RAMPACK_PARAMETERUPDATERFACTORY_H

#include <memory>

#include "core/ParameterUpdater.h"


class ParameterUpdaterFactory {
public:
    static std::unique_ptr<ParameterUpdater> create(std::string updaterString);
};


#endif //RAMPACK_PARAMETERUPDATERFACTORY_H
