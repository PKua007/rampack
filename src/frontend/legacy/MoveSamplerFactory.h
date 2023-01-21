//
// Created by pkua on 21.03.2022.
//

#ifndef RAMPACK_MOVESAMPLERFACTORY_H
#define RAMPACK_MOVESAMPLERFACTORY_H

#include <memory>

#include "core/MoveSampler.h"
#include "core/ShapeTraits.h"


namespace legacy {
    class MoveSamplerFactory {
    public:
        static std::unique_ptr<MoveSampler> create(const std::string &moveSamplerString);
    };
}


#endif //RAMPACK_MOVESAMPLERFACTORY_H
