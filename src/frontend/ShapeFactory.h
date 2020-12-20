//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_SHAPEFACTORY_H
#define RAMPACK_SHAPEFACTORY_H

#include <memory>
#include <string>

#include "core/ShapeTraits.h"

class ShapeFactory {
public:
    static std::unique_ptr<ShapeTraits> shapeTraitsFor(const std::string &shapeName, const std::string &shapeAttributes,
                                                       const std::string &interaction);
};


#endif //RAMPACK_SHAPEFACTORY_H
