//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_SPHERE_H
#define RAMPACK_SPHERE_H

#include "Shape.h"

class Sphere : public Shape {
private:
    double radius{};

public:
    Sphere() : radius{1} { }
    explicit Sphere(double radius);
    ~Sphere() {

    }

    [[nodiscard]] bool overlap(const Shape &other, double scaleFactor, const BoundaryConditions &bc) const override;
    [[nodiscard]] std::unique_ptr<Shape> clone() const override;
};


#endif //RAMPACK_SPHERE_H
