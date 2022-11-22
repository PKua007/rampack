//
// Created by Piotr Kubala on 20/12/2020.
//

#ifndef RAMPACK_SHAPEPRINTER_H
#define RAMPACK_SHAPEPRINTER_H

#include <string>
#include <stdexcept>

#include "Shape.h"


/**
 * @brief Exception thrown when given ShapePrinter format does not exist.
 */
class NoSuchShapePrinterException : public std::runtime_error {
public:
    explicit NoSuchShapePrinterException(const std::string &what) : runtime_error(what) { }
};

/**
 * @brief An interface for printing the shape in supported formats.
 */
class ShapePrinter {
public:
    virtual ~ShapePrinter() = default;

    [[nodiscard]] virtual std::string print(const Shape &shape) const = 0;
};

#endif //RAMPACK_SHAPEPRINTER_H
