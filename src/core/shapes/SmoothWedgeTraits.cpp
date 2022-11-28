//
// Created by ciesla on 8/9/22.
//

#include "SmoothWedgeTraits.h"
#include "utils/Assertions.h"
#include "geometry/xenocollide/XCBodyBuilder.h"


double SmoothWedgeTraits::getVolume(double R, double r, double l) {
    double r2 = r*r;
    double r3 = r2*r;
    double R2 = R*R;
    double R3 = R2*R;
    double Rr = R*r;
    double epsilon{};
    // Prevent nan if R == r and l == 0
    if (R == r)
        epsilon = 0;
    else
        epsilon = (R - r)/l;
    double epsilon2 = epsilon*epsilon;

    return M_PI*l/3*(R2 + Rr + r2)*(1 + epsilon2) + 2*M_PI/3*(R3 + r3);
}

SmoothWedgeTraits::SmoothWedgeTraits(double R, double r, double l, std::size_t subdivisions)
        : XenoCollideTraits({0, 0, 1}, {1, 0, 0}, {0, 0, 0},
                            SmoothWedgeTraits::getVolume(R, r, l),
                            {{"beg", {0, 0, (-l + R - r)/2}}, {"end", {0, 0, (l + R - r)/2}}}),
          R{R}, r{r}, l{l}
{
    Expects(r > 0);
    Expects(R > 0);
    Expects(l >= std::abs(R - r));

    if (subdivisions == 0 || subdivisions == 1) {
        this->interactionCentres = {};
        this->shapeModel.emplace_back(R, r, l);
        return;
    }

    std::vector<double> alphas = this->calculateRelativeSpherePositions(subdivisions);

    double begZ = (-this->l + this->R - this->r)/2;
    this->shapeModel.reserve(subdivisions);
    this->interactionCentres.reserve(subdivisions);
    for (std::size_t i{}; i < subdivisions; i++) {
        double alpha0 = alphas[i];
        double alpha1 = alphas[i + 1];

        double r0 = alpha0*this->r + (1 - alpha0)*this->R;
        double r1 = alpha1*this->r + (1 - alpha1)*this->R;
        double l01 = (alpha1 - alpha0)*this->l;

        this->shapeModel.emplace_back(r0, r1, l01);

        double centreZ = begZ + this->l*(alpha0 + alpha1)/2 - (r0 - r1)/2;
        this->interactionCentres.push_back({0, 0, centreZ});
    }
}

std::vector<double> SmoothWedgeTraits::calculateRelativeSpherePositions(std::size_t subdivisions) const {
    double A{};
    // Prevent nan if R == r and l == 0
    if (this->R == this->r)
        A = 1;
    else
        A = (this->l - this->r + this->R)/(this->l + this->r - this->R);

    std::vector<double> alphas;
    alphas.reserve(subdivisions + 1);
    alphas.push_back(0);
    for (std::size_t i{}; i < subdivisions; i++)
        alphas.push_back(A*alphas.back() + 1);
    for (auto &alpha : alphas)
        alpha /= alphas.back();
    return alphas;
}

SmoothWedgeTraits::CollideGeometry::CollideGeometry(double R, double r, double l)
        : R{R}, r{r}, l{l}, Rminusr{R - r}, Rpos{(-l + R - r)/2}, rpos{(l + R - r)/2},
          circumsphereRadius{(l + R + r)/2}, insphereRadius{0.5*(R + r + Rminusr*Rminusr/l)}
{
    Expects(R > 0);
    Expects(r > 0);
    Expects(l > 0);
}
