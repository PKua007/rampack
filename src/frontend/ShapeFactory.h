//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_SHAPEFACTORY_H
#define RAMPACK_SHAPEFACTORY_H

#include <memory>
#include <string>

#include "core/ShapeTraits.h"
#include "utils/Version.h"


namespace legacy {
    class ShapeFactory {
    public:
        static std::shared_ptr<ShapeTraits> shapeTraitsFor(const std::string &shapeName,
                                                           const std::string &shapeAttributes,
                                                           const std::string &interaction,
                                                           const Version &version = "0.5.0");
    };
}


#endif //RAMPACK_SHAPEFACTORY_H
