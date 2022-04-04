//
// Created by pkua on 04.04.2022.
//

#ifndef RAMPACK_EULERANGLES_H
#define RAMPACK_EULERANGLES_H

#include <array>

#include "Matrix.h"


/**
 * @brief A class converting rotation matrix to external XYZ Euler angles (Tait-Brian angles).
 * @details <p> Excluding situations with a gimbal lock (y angle is +/- pi/2), there are always 2 solutions stored in
 * EulerAngles::first and EulerAngles::second. When a gimbal lock occurs, there is an infinite number of solutions.
 * In this implementation, then, both solutions are equal with x angle is set to 0.
 *
 * <p> Numerical errors are the highest near a gimbal lock, where the elements of the matrix reconstructed from
 * calculated angles can be off by up to 10^-8. Far from a gimbal lock, the accuracy is closer to double type precision.
 * Gimbal lock is numerically detected until the deviation of y angle from +/- pi is around 1e-8 - this is because
 * double-precision sine is exactly 1 for smaller deviations.
 */
class EulerAngles {
private:
    static bool isRotationMatrix(const Matrix<3, 3> &matrix);
    static std::pair<std::array<double, 3>, std::array<double, 3>> forMatrix(const Matrix<3, 3> &matrix);

public:
    static constexpr double EPSILON = 1e-12;

    /** @brief First set of Euler angles. The order of angles is: X, Y, Z. */
    const std::array<double, 3> first{};

    /** @brief Second set of Euler angles. The order of angles is: X, Y, Z. */
    const std::array<double, 3> second{};

    /**
     * @brief Constructs Euler angles for a given matrix.
     */
    explicit EulerAngles(const Matrix<3, 3> &matrix);

    [[nodiscard]] bool hasGimbalLock() const;
};


#endif //RAMPACK_EULERANGLES_H
