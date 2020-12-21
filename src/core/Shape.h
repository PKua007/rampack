//
// Created by Piotr Kubala on 12/12/2020.
//

#ifndef RAMPACK_SHAPE_H
#define RAMPACK_SHAPE_H

#include <memory>
#include <iosfwd>

#include "BoundaryConditions.h"
#include "geometry/Vector.h"
#include "geometry/Matrix.h"

class Shape {
private:
    Vector<3> position{};
    Matrix<3, 3> orientation{};

public:
    Shape() : orientation{Matrix<3, 3>::identity()} { }
    explicit Shape(const Vector<3> &position) : position{position}, orientation{Matrix<3, 3>::identity()} { }
    Shape(const Vector<3> &position, const Matrix<3, 3> &orientation) : position{position}, orientation{orientation} { }

    void translate(const Vector<3> &translation, const BoundaryConditions &bc);
    void rotate(const Matrix<3, 3> &rotation);
    void applyBCTranslation(const BoundaryConditions &bc, Shape &other) const;
    [[nodiscard]] const Vector<3> &getPosition() const { return this->position; }
    [[nodiscard]] const Matrix<3, 3> &getOrientation() const { return this->orientation; }
};


#endif //RAMPACK_SHAPE_H
