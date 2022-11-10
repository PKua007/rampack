//
// Created by pkua on 22.02.2022.
//

#ifndef RAMPACK_TRICLINICBOXSCALERFACTORY_H
#define RAMPACK_TRICLINICBOXSCALERFACTORY_H

#include <memory>
#include <string>

#include "core/TriclinicBoxScaler.h"

class TriclinicBoxScalerFactory {
public:
    [[nodiscard]] static std::unique_ptr<TriclinicBoxScaler> create(const std::string &scalingType,
                                                                    double initialStepSize);
};


#endif //RAMPACK_TRICLINICBOXSCALERFACTORY_H
