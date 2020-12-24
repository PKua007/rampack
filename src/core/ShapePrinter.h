//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SHAPEPRINTER_H
#define RAMPACK_SHAPEPRINTER_H

#include <string>

#include "Shape.h"

class ShapePrinter {
public:
    virtual ~ShapePrinter() = default;

    [[nodiscard]] virtual std::string toWolfram(const Shape &shape) const = 0;
};

#endif //RAMPACK_SHAPEPRINTER_H
