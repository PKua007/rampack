//
// Created by Piotr Kubala on 16/12/2020.
//

#ifndef RAMPACK_SPHEROCYLINDER_H
#define RAMPACK_SPHEROCYLINDER_H

#include "core/Shape.h"

class Spherocylinder : public Shape {
private:
    double length{};    // distance between two spherical caps centres
    double radius{};    // radius of spherical caps

    static constexpr double EPSILON = 0.0000000001;

    [[nodiscard]] Vector<3> getEnd(short beginOrEnd, double scale) const;
    [[nodiscard]] double distanceFrom(const Spherocylinder &s, double scale) const;

public:
    Spherocylinder() : length{1}, radius{1} { }
    Spherocylinder(double length, double radius) : length(length), radius(radius) { }

    [[nodiscard]] bool overlap(const Shape &other, double scaleFactor, const BoundaryConditions &bc) const override;
    [[nodiscard]] std::unique_ptr<Shape> clone() const override;
    [[nodiscard]] double getVolume() const override;
    [[nodiscard]] std::string toWolfram(double scaleFactor) const override;
};


#endif //RAMPACK_SPHEROCYLINDER_H
