//
// Created by pkua on 04.04.2022.
//

#include <tuple>
#include <cmath>

#include "EulerAngles.h"
#include "utils/Exceptions.h"

// Based on: http://eecs.qmul.ac.uk/~gslabaugh/publications/euler.pdf
std::pair<std::array<double, 3>, std::array<double, 3>> EulerAngles::forMatrix(const Matrix<3, 3> &matrix) {
    double R11 = matrix(0, 0);
    double R12 = matrix(0, 1);
    double R13 = matrix(0, 2);
    double R21 = matrix(1, 0);
    double R31 = matrix(2, 0);
    double R32 = matrix(2, 1);
    double R33 = matrix(2, 2);

    double psi1{}, theta1{}, phi1{};
    double psi2{}, theta2{}, phi2{};

    if (R31 > -1 + EPSILON || R31 < 1 - EPSILON) {
        theta1 = -std::asin(R31);
        theta2 = M_PI - theta1;
        double c1 = std::cos(theta1);
        double c2 = std::cos(theta2);
        psi1 = std::atan2(R32/c1, R33/c1);
        psi2 = std::atan2(R32/c2, R33/c2);
        phi1 = std::atan2(R21/c1, R11/c1);
        phi2 = std::atan2(R21/c2, R11/c2);
    } else {
        phi1 = phi2 = 0;
        if (R31 < 0) {
            theta1 = theta2 = M_PI/2;
            psi1 = psi2 = std::atan2(R12, R13);
        } else {
            theta1 = theta2 = -M_PI/2;
            psi1 = psi2 = std::atan2(-R12, -R13);
        }
    }

    return {{psi1, theta1, phi1}, {psi2, theta2, phi2}};
}

EulerAngles::EulerAngles(const Matrix<3, 3> &matrix) {
    Expects(EulerAngles::isRotationMatrix(matrix));

    auto &firstMutable = const_cast<std::array<double, 3> &>(this->first);
    auto &secondMutable = const_cast<std::array<double, 3> &>(this->second);
    std::tie(firstMutable, secondMutable) = EulerAngles::forMatrix(matrix);
}

bool EulerAngles::hasGimbalLock() const {
    return std::abs(this->first[1]) == M_PI/2;
}

bool EulerAngles::isRotationMatrix(const Matrix<3, 3> &matrix) {
    Matrix<3, 3> shouldBeZero = matrix * matrix.transpose() - Matrix<3, 3>::identity();
    for (std::size_t i{}; i < 3; i++)
        for (std::size_t j{}; j < 3; j++)
            if (std::abs(shouldBeZero(i, j)) >= EPSILON)
                return false;
    return true;
}
