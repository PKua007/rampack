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

/**
 * @brief A class representing a 3D shape with its position and orientation.
 * @details The shape is a simple, non-virtual class - concrete shapes with overlap methods and interaction potentials
 * are acutally implemented using ShapeTraits class.
 */
class Shape {
private:
    Vector<3> position{};
    Matrix<3, 3> orientation{};

public:
    /**
     * @brief Default constructor creating a shape placed in the origin with a default orientation.
     */
    Shape() : orientation{Matrix<3, 3>::identity()} { }

    /**
     * @brief Constructor of a default-oriented shape placed in @a position.
     */
    explicit Shape(const Vector<3> &position) : position{position}, orientation{Matrix<3, 3>::identity()} { }

    /**
     * @brief Constructor taking both the position and orientation of a shape.
     */
    Shape(const Vector<3> &position, const Matrix<3, 3> &orientation) : position{position}, orientation{orientation} { }

    /**
     * @brief Translates the shape by @a translation vector respecting the boundary conditions @a bc.
     */
    void translate(const Vector<3> &translation, const BoundaryConditions &bc);

    void setPosition(const Vector<3> &position_);

    /**
     * @brief Scales all coordinates of the position vector by a common factor @a factor.
     */
    void scale(double factor);

    /**
     * @brief Performs elementwise scaling of the position vector coordinates by @a factor.
     */
    void scale(const std::array<double, 3> &factor);

    /**
     * @brief Applies the rotation @a rotation to molecule's orientation.
     */
    void rotate(const Matrix<3, 3> &rotation);

    void setOrientation(const Matrix<3, 3> &orientation_);

    /**
     * @brief Applies boundary conditions @a bc translation to @a other shape moving it near this shape.
     */
    void applyBCTranslation(const BoundaryConditions &bc, Shape &other) const;

    [[nodiscard]] const Vector<3> &getPosition() const { return this->position; }
    [[nodiscard]] const Matrix<3, 3> &getOrientation() const { return this->orientation; }

    friend bool operator==(const Shape &lhs, const Shape &rhs) {
        return std::tie(lhs.position, lhs.orientation) == std::tie(rhs.position, rhs.orientation);
    }

    friend bool operator!=(const Shape &lhs, const Shape &rhs) {
        return !(rhs == lhs);
    }

    /**
     * @brief Stream insertion operator for debugging printing a textual representation of the shape.
     */
    friend std::ostream &operator<<(std::ostream &out, const Shape &shape);
};


#endif //RAMPACK_SHAPE_H
