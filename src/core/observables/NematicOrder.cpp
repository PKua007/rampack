//
// Created by Piotr Kubala on 23/03/2021.
//

#include <algorithm>

#include "NematicOrder.h"

void NematicOrder::calculate(const Packing &packing, [[maybe_unused]] double temperature,
                             [[maybe_unused]] double pressure, const ShapeTraits &shapeTraits)
{
    this->QTensor = Matrix<3, 3>{};
    for (const auto &shape : packing) {
        Vector<3> axis = shapeTraits.getGeometry().getPrimaryAxis(shape);
        for (std::size_t i{}; i < 3; i++)
            for (std::size_t j{}; j < 3; j++)
                this->QTensor(i, j) += axis[i] * axis[j];
    }
    this->QTensor /= static_cast<double>(packing.size());
    // Take into account the normalisation of the order parameter
    this->QTensor = 0.5*(3.*this->QTensor - Matrix<3, 3>::identity());
    auto eigenvalues = calculateEigenvalues(this->QTensor);

    // P2 is given by the eigenvalue with the highest magnitude
    this->P2 = *std::max_element(eigenvalues.begin(), eigenvalues.end(), [](double eigval1, double eigval2) {
        return std::abs(eigval1) < std::abs(eigval2);
    });
}

std::array<double, 3> NematicOrder::calculateEigenvalues(const Matrix<3, 3> &matrix) {
    // A trival method from arXiv:1306.6291 based on the Cardano equation

    double A11 = matrix(0, 0);
    double A12 = matrix(0, 1);
    double A13 = matrix(0, 2);
    double A22 = matrix(1, 1);
    double A23 = matrix(1, 2);
    double A33 = matrix(2, 2);

    double b = A11 + A22 + A33;
    double c = A11*A22 + A11*A33 + A22*A33 - A12*A12 - A13*A13 - A23*A23;
    double d = A11*A23*A23 + A22*A13*A13 + A33*A12*A12 - A11*A22*A33 - 2*A12*A13*A23;

    double p = b*b - 3*c;
    Assert(p > 0);
    double sqrt_p = std::sqrt(p);
    double q = 2*b*b*b - 9*b*c - 27*d;
    double ratio = q / 2 / (sqrt_p*sqrt_p*sqrt_p);

    if (std::abs(ratio) > 1) {
        Assert(std::abs(ratio) <= 1 + 1e-15);
        ratio = ratio < 0 ? -1 : 1;
    }

    double Delta = std::acos(ratio);

    return {1./3*(b + 2*sqrt_p*std::cos(Delta/3)),
            1./3*(b + 2*sqrt_p*std::cos((Delta + 2*M_PI)/3)),
            1./3*(b + 2*sqrt_p*std::cos((Delta - 2*M_PI)/3))};
}

std::vector<std::string> NematicOrder::getIntervalHeader() const {
    if (this->dumpQTensor)
        return {"P2", "Q_11", "Q_12", "Q_13", "Q_22", "Q_23", "Q_33"};
    else
        return {"P2"};
}

std::vector<double> NematicOrder::getIntervalValues() const {
    if (this->dumpQTensor) {
        return {this->P2, this->QTensor(0, 0), this->QTensor(0, 1), this->QTensor(0, 2), this->QTensor(1, 1),
                this->QTensor(1, 2), this->QTensor(2, 2)};
    } else {
        return {this->P2};
    }
}
