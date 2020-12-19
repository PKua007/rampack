//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_SPHERE_H
#define RAMPACK_SPHERE_H

#include "core/Shape.h"
#include "core/HardShape.h"

class Sphere : public Shape, public HardShape {
private:
    double radius{};

public:
    Sphere() : radius{1} { }
    explicit Sphere(double radius);

    [[nodiscard]] bool overlap(const HardShape &other, double scaleFactor, const BoundaryConditions &bc) const override;
    [[nodiscard]] std::unique_ptr<Shape> clone() const override;
    [[nodiscard]] std::string toWolfram(double scaleFactor) const override;
    [[nodiscard]] double getVolume() const override;

    [[nodiscard]] double getRadius() const { return this->radius; }
};


#endif //RAMPACK_SPHERE_H
