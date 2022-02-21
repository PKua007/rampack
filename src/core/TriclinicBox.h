//
// Created by pkua on 02.02.2022.
//

#ifndef RAMPACK_TRICLINICBOX_H
#define RAMPACK_TRICLINICBOX_H

#include <vector>
#include <array>

#include "geometry/Vector.h"
#include "geometry/Matrix.h"


/**
 * @brief The class representing triclinic (parallelpiped) simulation box.
 */
class TriclinicBox {
private:
    Matrix<3, 3> dimensions;
    Matrix<3, 3> inverseDimensions;

public:
    /**
     * @brief The default box: 1 x 1 x 1 cube.
     */
    TriclinicBox() : TriclinicBox(1) { }

    /**
     * @brief The constructor creating the cube with side length @a linearSize
     */
    explicit TriclinicBox(double linearSize)
            : TriclinicBox(std::array<double, 3>{linearSize, linearSize, linearSize})
    { }

    /**
     * @brief The constructor creating the cuboid with side lengths @a dimensions
     */
    explicit TriclinicBox(const std::array<double, 3> &dimensions)
            : TriclinicBox(Matrix<3, 3>{dimensions[0], 0, 0,
                                        0, dimensions[1], 0,
                                        0, 0, dimensions[2]})
    { }

    /**
     * @brief The constructor creating the box with side vectors given by matrix @a dimensions columns.
     */
    explicit TriclinicBox(const Matrix<3, 3> &dimensions)
            : dimensions{dimensions}, inverseDimensions{dimensions.inverse()}
    { }

    /**
     * @brief The constructor creating the box with side vectors given by elements of @a dimensions array.
     */
    explicit TriclinicBox(const std::array<Vector<3>, 3> &dimensions);

    /**
     * @brief Translates absolute (lab) coordinates to box-relative (having the range of [0, 1])
     */
    [[nodiscard]] Vector<3> absoluteToRelative(const Vector<3> &pos) const {
        return this->inverseDimensions * pos;
    }

    /**
     * @brief Translates box-relative coordinates (having the range of [0, 1]) to absolute (lab) ones
     */
    [[nodiscard]] Vector<3> relativeToAbsolute(const Vector<3> &pos) const {
        return this->dimensions * pos;
    }

    /**
     * @brief Returns the dimensions matrix (columns are box side vectors).
     */
    [[nodiscard]] const Matrix<3, 3> &getDimensions() const { return dimensions; }

    /**
     * @brief Transforms the box by @a transformation matrix linear transformation.
     */
    void transform(const Matrix<3, 3> &transformation);

    /**
     * @brief Performs box scaling by diagonal scaling matrix given by @a factors.
     */
    void scale(const std::array<double, 3> &factors);

    /**
     * @brief Scales the box by given linear factor @a factor.
     */
    void scale(double factor) { this->scale(std::array<double, 3>{factor, factor, factor}); }

    /**
     * @brief Returns box side vectors.
     */
    [[nodiscard]] std::array<Vector<3>, 3> getSides() const;

    /**
     * @brief Returns box volume (signed).
     */
    [[nodiscard]] double getVolume() const;

    /**
     * @brief Returns heights of the box.
     * @details The first height is in the direction orthogonal to plane spanned by the second and the third sides.
     * The rest of heights are computed similarly by a cyclic permutation of sides.
     */
    [[nodiscard]] std::array<double, 3> getHeights() const;
};


#endif //RAMPACK_TRICLINICBOX_H
