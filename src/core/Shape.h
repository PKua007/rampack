//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_SHAPE_H
#define RAMPACK_SHAPE_H

#include <array>
#include <memory>
#include <iosfwd>

#include "BoundaryConditions.h"

class Shape {
private:
    std::array<double, 3> position{};

public:
    Shape() { this->position.fill(0); };
    explicit Shape(const std::array<double, 3> &position) : position{position} { }
    virtual ~Shape() = default;

    void translate(const std::array<double, 3> &translation, const BoundaryConditions &bc);
    [[nodiscard]] std::array<double, 3> getPosition() const { return this->position; }
    [[nodiscard]] virtual bool overlap(const Shape &other, double scaleFactor, const BoundaryConditions &bc) const = 0;
    [[nodiscard]] virtual std::unique_ptr<Shape> clone() const = 0;
    [[nodiscard]] virtual double getVolume() const = 0;
    [[nodiscard]] virtual std::string toWolfram(double scaleFactor) const = 0;
};


#endif //RAMPACK_SHAPE_H
