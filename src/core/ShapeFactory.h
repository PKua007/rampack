//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_SHAPEFACTORY_H
#define RAMPACK_SHAPEFACTORY_H

#include <memory>
#include <string>

#include "Shape.h"

class ShapeFactory {
public:
    static std::unique_ptr<Shape> createShape(const std::string &shapeName, const std::string &shapeAttributes);
};


#endif //RAMPACK_SHAPEFACTORY_H
