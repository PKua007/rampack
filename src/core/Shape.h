//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_SHAPE_H
#define RAMPACK_SHAPE_H

#include <memory>
#include <iosfwd>

#include "BoundaryConditions.h"
#include "geometry/Vector.h"

class Shape {
private:
    Vector<3> position{};

public:
    Shape() = default;
    explicit Shape(const Vector<3> &position) : position{position} { }
    virtual ~Shape() = default;

    void translate(const Vector<3> &translation, const BoundaryConditions &bc);
    [[nodiscard]] const Vector<3> &getPosition() const { return this->position; }
    [[nodiscard]] virtual bool overlap(const Shape &other, double scaleFactor, const BoundaryConditions &bc) const = 0;
    [[nodiscard]] virtual std::unique_ptr<Shape> clone() const = 0;
    [[nodiscard]] virtual double getVolume() const = 0;
    [[nodiscard]] virtual std::string toWolfram(double scaleFactor) const = 0;
};


#endif //RAMPACK_SHAPE_H
