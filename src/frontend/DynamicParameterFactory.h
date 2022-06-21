//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_DYNAMICPARAMETERFACTORY_H
#define RAMPACK_DYNAMICPARAMETERFACTORY_H

#include <memory>

#include "core/DynamicParameter.h"


class DynamicParameterFactory {
public:
    static std::unique_ptr<DynamicParameter> create(std::string updaterString);
};


#endif //RAMPACK_DYNAMICPARAMETERFACTORY_H
