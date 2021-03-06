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
    void setPosition(const Vector<3> &position_);
    void scale(double factor);
    void scale(const std::array<double, 3> &factor);
    void rotate(const Matrix<3, 3> &rotation);
    void setOrientation(const Matrix<3, 3> &orientation_);
    void applyBCTranslation(const BoundaryConditions &bc, Shape &other) const;
    [[nodiscard]] const Vector<3> &getPosition() const { return this->position; }
    [[nodiscard]] const Matrix<3, 3> &getOrientation() const { return this->orientation; }

    friend bool operator==(const Shape &lhs, const Shape &rhs) {
        return std::tie(lhs.position, lhs.orientation) == std::tie(rhs.position, rhs.orientation);
    }

    friend bool operator!=(const Shape &lhs, const Shape &rhs) {
        return !(rhs == lhs);
    }

    friend std::ostream &operator<<(std::ostream &out, const Shape &shape);
};


#endif //RAMPACK_SHAPE_H
