//
// Created by Piotr Kubala on 23/03/2021.
//

#include <algorithm>

#include "NematicOrder.h"

void NematicOrder::calculate(const Packing &packing, [[maybe_unused]] double temperature,
                             [[maybe_unused]] double pressure, const ShapeTraits &shapeTraits)
{
    Matrix<3, 3> Qtensor;
    for (const auto &shape : packing) {
        Vector<3> axis = shapeTraits.getPrimaryAxis(shape);
        for (std::size_t i{}; i < 3; i++)
            for (std::size_t j{}; j < 3; j++)
                Qtensor(i, j) += axis[i] * axis[j];
    }
    Qtensor /= static_cast<double>(packing.size());

    auto eigenvalues = calculateEigenvalues(Qtensor);
    // Take into account the normalisation of the order parameter
    for (auto &eigenvalue : eigenvalues)
        eigenvalue = 0.5*(3*eigenvalue - 1);

    // P2 is given by the eigenvalue with the highest magnitude
    this->P2 = *std::max_element(eigenvalues.begin(), eigenvalues.end(), [](double eigval1, double eigval2) {
        return std::abs(eigval1) < std::abs(eigval2);
    });
}

std::array<double, 3> NematicOrder::calculateEigenvalues(const Matrix<3, 3> &tensor) {
    // A trival method from arXiv:1306.6291 based on the Cardano equation

    double A11 = tensor(0, 0);
    double A12 = tensor(0, 1);
    double A13 = tensor(0, 2);
    double A22 = tensor(1, 1);
    double A23 = tensor(1, 2);
    double A33 = tensor(2, 2);

    double b = A11 + A22 + A33;
    double c = A11*A22 + A11*A33 + A22*A33 - A12*A12 - A13*A13 - A23*A23;
    double d = A11*A23*A23 + A22*A13*A13 + A33*A12*A12 - A11*A22*A33 - 2*A12*A13*A23;

    double p = b*b - 3*c;
    Assert(p > 0);
    double sqrt_p = std::sqrt(p);
    double q = 2*b*b*b - 9*b*c - 27*d;
    double ratio = q / 2 / (sqrt_p*sqrt_p*sqrt_p);
    Assert(std::abs(ratio) <= 1);
    double Delta = std::acos(ratio);

    return {1./3*(b + 2*sqrt_p*std::cos(Delta/3)),
            1./3*(b + 2*sqrt_p*std::cos((Delta + 2*M_PI)/3)),
            1./3*(b + 2*sqrt_p*std::cos((Delta - 2*M_PI)/3))};
}
