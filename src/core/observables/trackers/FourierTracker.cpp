//
// Created by pkua on 20.10.22.
//

#include <algorithm>
#include <Eigen/Core>
#include <root_finder/root_finder.hpp>

#include "FourierTracker.h"
#include "utils/Assertions.h"
#include "core/PeriodicBoundaryConditions.h"


std::string FourierTracker::getTrackingMethodName() const {
    return this->functionName + "_fourier";
}

void FourierTracker::calculateOrigin(const Packing &packing, const ShapeTraits &shapeTraits) {
    FourierCoefficients fourierCoefficients = this->calculateFourierCoefficients(packing, shapeTraits);
    Vector<3> originPosRel = this->calculateRelativeOriginPos(fourierCoefficients);
    originPosRel = this->normalizeOriginPos(originPosRel);
    this->originPos = packing.getBox().relativeToAbsolute(originPosRel);
}

Vector<3> FourierTracker::calculateRelativeOriginPos(const FourierTracker::FourierCoefficients &coefficients) const {
    switch (this->nonzeroWavenumberIdxs.size()) {
        case 1:
            return this->calculateRelativeOriginPos1D(coefficients);
        case 2:
            return this->calculateRelativeOriginPos2D(coefficients);
        case 3:
            return this->calculateRelativeOriginPos3D(coefficients);
        default:
            throw AssertionException("FourierTracker: zeros = " + std::to_string(this->nonzeroWavenumberIdxs.size()));
    }
}

Vector<3> FourierTracker::calculateRelativeOriginPos1D(const FourierTracker::FourierCoefficients &coefficients) const {
    std::size_t nonzeroIdx = this->nonzeroWavenumberIdxs.front();
    std::array<std::size_t, 3> idxs{};
    idxs[nonzeroIdx] = COS;
    double c = coefficients[idxs[0]][idxs[1]][idxs[2]];
    idxs[nonzeroIdx] = SIN;
    double d = coefficients[idxs[0]][idxs[1]][idxs[2]];

    if (c*c + d*d < AMPLITUDE_EPSILON*AMPLITUDE_EPSILON)
        return {};

    auto wavenumber = static_cast<double>(this->wavenumbers[nonzeroIdx]);
    Vector<3> relPos{};
    relPos[nonzeroIdx] = std::fmod(std::atan2(d, c)/wavenumber/2/M_PI + 1, 1);
    return relPos;
}

Vector<3> FourierTracker::calculateRelativeOriginPos2D(const FourierTracker::FourierCoefficients &coefficients) const {
    std::size_t nonzeroIdx1 = this->nonzeroWavenumberIdxs[0];
    std::size_t nonzeroIdx2 = this->nonzeroWavenumberIdxs[1];
    std::array<std::size_t, 3> idxs{};
    idxs[nonzeroIdx1] = COS;
    idxs[nonzeroIdx2] = COS;
    double A = coefficients[idxs[0]][idxs[1]][idxs[2]];
    idxs[nonzeroIdx1] = COS;
    idxs[nonzeroIdx2] = SIN;
    double B = coefficients[idxs[0]][idxs[1]][idxs[2]];
    idxs[nonzeroIdx1] = SIN;
    idxs[nonzeroIdx2] = COS;
    double C = coefficients[idxs[0]][idxs[1]][idxs[2]];
    idxs[nonzeroIdx1] = SIN;
    idxs[nonzeroIdx2] = SIN;
    double D = coefficients[idxs[0]][idxs[1]][idxs[2]];

    if (A*A + B*B + C*C + D*D < AMPLITUDE_EPSILON*AMPLITUDE_EPSILON)
        return {};

    // We express the function
    // (1)   A cx cy + B cx sy + C sx cy + D sx sy
    // as
    // (2)   sqrt((C - B)^2 + (A + D)^2) cos(x + y - a) + sqrt((C + B)^2 + (A - D)^2) cos(x - y - b)
    // where a and b are given by atan2-s as below. Equations
    // (3)   x + y == a, x - y == b
    // maximize both cosines, giving the final result. The amplitude is given by the sum of the two sqrt(...) in (2).
    double a = FourierTracker::guardedAtan2(C + B, A - D);
    double b = FourierTracker::guardedAtan2(C - B, A + D);
    auto wavenumber1 = static_cast<double>(this->wavenumbers[nonzeroIdx1]);
    auto wavenumber2 = static_cast<double>(this->wavenumbers[nonzeroIdx2]);
    Vector<3> relPos{};
    relPos[nonzeroIdx1] = std::fmod(((a + b)/2)/wavenumber1/2/M_PI + 1, 1);
    relPos[nonzeroIdx2] = std::fmod(((a - b)/2)/wavenumber2/2/M_PI + 1, 1);
    return relPos;
}

FourierTracker::FourierCoefficients FourierTracker::calculateFourierCoefficients(const Packing &packing,
                                                                                 const ShapeTraits &shapeTraits) const
{
    FourierCoefficients coefficients{};

    for (std::size_t i{}; i < this->fourierFunctions[0].size(); i++) {
        for (std::size_t j{}; j < this->fourierFunctions[1].size(); j++) {
            for (std::size_t k{}; k < this->fourierFunctions[2].size(); k++) {
                const auto &f0 = this->fourierFunctions[0][i];
                const auto &f1 = this->fourierFunctions[1][j];
                const auto &f2 = this->fourierFunctions[2][k];
                auto &coefficient = coefficients[i][j][k];

                auto fProduct = [&f0, &f1, &f2](const Vector<3> &pos) {
                    return f0(pos[0]) * f1(pos[1]) * f2(pos[2]);
                };

                coefficient = 0;
                for (std::size_t shapeI{}; shapeI < packing.size(); shapeI++) {
                    const auto &shape = packing[shapeI];
                    double functionMapping = this->function(shape, shapeTraits);
                    Vector<3> relativePos = packing.getBox().absoluteToRelative(shape.getPosition());
                    coefficient += fProduct(relativePos) * functionMapping;
                }
            }
        }
    }

    return coefficients;
}

FourierTracker::FourierTracker(const std::array<std::size_t, 3> &wavenumbers, Function function,
                               std::string functionName)
        : wavenumbers{wavenumbers}, function{std::move(function)}, functionName{std::move(functionName)}
{
    for (std::size_t i{}; i < 3; i++) {
        std::size_t wavenumber = this->wavenumbers[i];
        if (wavenumber > 0)
            this->nonzeroWavenumberIdxs.push_back(i);
    }
    Expects(!this->nonzeroWavenumberIdxs.empty());

    this->fillFourierFunctions();
}

void FourierTracker::fillFourierFunctions() {
    auto functionFiller = [](std::size_t wavenumber) -> std::vector<FourierFunction> {
        if (wavenumber == 0) {
            FourierFunction constant = [](double) -> double { return 1; };
            return {constant};
        } else {
            double k = 2*M_PI*static_cast<double>(wavenumber);
            FourierFunction sin = [k](double xi) { return std::sin(k*xi); };
            FourierFunction cos = [k](double xi) { return std::cos(k*xi); };
            return {cos, sin};
        }
    };

    std::transform(this->wavenumbers.begin(), this->wavenumbers.end(), this->fourierFunctions.begin(), functionFiller);
}

Vector<3> FourierTracker::normalizeOriginPos(const Vector<3> &originPosRel) {
    PeriodicBoundaryConditions pbc(1);
    Vector<3> bestOriginPosRel = originPosRel;
    double bestDeltaR2 = pbc.getDistance2(this->previousRelValue, originPosRel);
    std::array<std::size_t, 3> idxs{};
    for (idxs[0] = 0; idxs[0] < std::max(2*this->wavenumbers[0], 1ul); idxs[0]++) {
        for (idxs[1] = 0; idxs[1] < std::max(2*this->wavenumbers[1], 1ul); idxs[1]++) {
            for (idxs[2] = 0; idxs[2] < std::max(2*this->wavenumbers[2], 1ul); idxs[2]++) {
                // Only "even" indices move from maximum to maximum ("odd" ones move to minimum)
                if (std::accumulate(idxs.begin(), idxs.end(), 0ul) % 2 != 0)
                    continue;

                Vector<3> nextOriginPosRel = originPosRel;
                for (std::size_t i{}; i < 3; i++)
                    if (this->wavenumbers[i] > 0)
                        nextOriginPosRel[i] += static_cast<double>(idxs[i])/static_cast<double>(2*this->wavenumbers[i]);
                nextOriginPosRel += pbc.getCorrection(nextOriginPosRel);

                double newDeltaR2 = pbc.getDistance2(this->previousRelValue, nextOriginPosRel);
                if (newDeltaR2 < bestDeltaR2) {
                    bestDeltaR2 = newDeltaR2;
                    bestOriginPosRel = nextOriginPosRel;
                }
            }
        }
    }

    this->previousRelValue = bestOriginPosRel;
    return bestOriginPosRel;
}

double FourierTracker::guardedAtan2(double y, double x) {
    if (x*x + y*y < AMPLITUDE_EPSILON*AMPLITUDE_EPSILON)
        return 0;
    else
        return std::atan2(y, x);
}

Vector<3> FourierTracker::calculateRelativeOriginPos3D(const FourierTracker::FourierCoefficients &coefficients) const {
    double A = FourierTracker::getCoefficient(coefficients, {COS, COS, COS});
    double B = FourierTracker::getCoefficient(coefficients, {COS, COS, SIN});
    double C = FourierTracker::getCoefficient(coefficients, {COS, SIN, COS});
    double D = FourierTracker::getCoefficient(coefficients, {COS, SIN, SIN});
    double E = FourierTracker::getCoefficient(coefficients, {SIN, COS, COS});
    double F = FourierTracker::getCoefficient(coefficients, {SIN, COS, SIN});
    double G = FourierTracker::getCoefficient(coefficients, {SIN, SIN, COS});
    double H = FourierTracker::getCoefficient(coefficients, {SIN, SIN, SIN});

    if (A*A + B*B + C*C + D*D + E*E + F*F + G*G + H*H < AMPLITUDE_EPSILON*AMPLITUDE_EPSILON)
        return {};

    double K1 = (A + G)/2;
    double K2 = (A - G)/2;
    double L1 = (B + H)/2;
    double L2 = (B - H)/2;
    double M1 = (C + E)/2;
    double M2 = (C - E)/2;
    double N1 = (D + F)/2;
    double N2 = (D - F)/2;

    double U = K2*K2 + M1*M1;
    double V = K2*L2 + M1*N1;
    double W = L2*L2 + N1*N1;
    double X = K1*K1 + M2*M2;
    double Y = K1*L1 + M2*N2;
    double Z = L1*L1 + N2*N2;

    // The function:
    // (1)   A cx cy cz + B cx cy sz + C cx sy cz + D cx sy sz + E sx cy cz + F sx cy sz + G sx sy cz + H sx sy sz
    // is rewritten as
    // (2)   A'(z) cx cy + B'(z) cx sy + C'(z) sx cy + D'(z) sx sy
    // where the coefficients depend on cz, sz, for example A'(z) = A cz + B sz. Then, we optimize the amplitude of
    // the above function of (x, y), which is equal (see calculateRelativeOriginPos2D)
    // (3)   sqrt((K2 cz + L2 sz)^2 + (M1 cz + N1 sz)^2) + sqrt((K1 cz + L1 sz)^2 + (M2 cz + N2 sz)^2).
    // We then equate the derivative over z to 0 and perform algebraic operations to write the equations as a 6-th
    // degree polynomial in cz and sz. We divide it by cz^6 and obtain 6-th degree polynomial over tan(z). It then
    // can be solved numericaly ...
    std::array<double, 7> polyCoef = this->calculateTanZCoefficients(U, V, W, X, Y, Z);
    Eigen::VectorXd eigenCoef(7);
    std::copy(polyCoef.rbegin(), polyCoef.rend(), eigenCoef.begin());
    constexpr auto INF = std::numeric_limits<double>::infinity();
    auto roots = RootFinder::solvePolynomial(eigenCoef, -INF, INF, 1e-12);
    // ... and we select the solution giving the largest amplitude (3) - this solves the problem for z value. ...
    auto[sz, cz] = this->findBestSinCosZ(roots, U, V, W, X, Y, Z);
    double z0 = FourierTracker::guardedAtan2(sz, cz);

    // ... We then proceed to optimize x and y in the same way as in calculateRelativeOriginPos2D.
    double A2D = A*cz + B*sz;
    double B2D = C*cz + D*sz;
    double C2D = E*cz + F*sz;
    double D2D = G*cz + H*sz;
    double a = FourierTracker::guardedAtan2(C2D + B2D, A2D - D2D);
    double b = FourierTracker::guardedAtan2(C2D - B2D, A2D + D2D);
    double x0 = (a + b)/2;
    double y0 = (a - b)/2;

    Vector<3> relPos{};
    relPos[0] = std::fmod(x0/static_cast<double>(this->wavenumbers[0])/2/M_PI + 1, 1);
    relPos[1] = std::fmod(y0/static_cast<double>(this->wavenumbers[1])/2/M_PI + 1, 1);
    relPos[2] = std::fmod(z0/static_cast<double>(this->wavenumbers[2])/2/M_PI + 1, 1);
    return relPos;
}

double FourierTracker::getCoefficient(const FourierTracker::FourierCoefficients &coefficients,
                                      const std::array<std::size_t, 3> &idxs)
{
    return coefficients[idxs[0]][idxs[1]][idxs[2]];
}

std::array<double, 7> FourierTracker::calculateTanZCoefficients(double U, double V, double W, double X, double Y,
                                                                double Z) const
{
    return {
        V*V*X - U*Y*Y,
        2*V*(W*X + (V-Y)*Y) - 2*U*(V*X + Y*(-X+Z)),
        -2*V*V*X + (U-W)*(U-W)*X + 4*V*(-U+W)*Y + 2*U*Y*Y - W*Y*Y + 4*V*Y*(X-Z) - U*(X-Z)*(X-Z) + V*V*Z,
        2*(V*(U-W)*X - 2*V*V*Y + (U-W)*(U-W)*Y + 2*V*Y*Y + W*Y*(X-Z) - V*(X-Z)*(X-Z) + V*(-U+W)*Z + U*Y*(-X+Z)),
        V*V*X + 4*V*(U-W)*Y - U*Y*Y + 2*W*Y*Y - W*(X-Z)*(X-Z) - 2*V*V*Z + (U-W)*(U-W)*Z + 4*V*Y*(-X+Z),
        2*(V*V*Y + W*Y*(-X+Z) - V*(Y*Y + (-U+W)*Z)),
        -W*Y*Y + V*V*Z
    };
}

std::pair<double, double> FourierTracker::findBestSinCosZ(const std::set<double> &roots, double U, double V, double W,
                                                          double X, double Y, double Z) const
{
    double bestSz{};
    double bestCz{};
    double bestVal{};

    for (double z : roots) {
        double norm = std::sqrt(1 + z*z);
        double cz = 1/norm;
        double sz = z/norm;
        double val = std::sqrt(U*cz*cz + 2*V*sz*cz + W*sz*sz) + std::sqrt(X*cz*cz + 2*Y*sz*cz + Z*sz*sz);
        if (val > bestVal) {
            bestVal = val;
            bestSz = sz;
            bestCz = cz;
        }
    }

    return {bestSz, bestCz};
}

void FourierTracker::reset() {
    this->previousRelValue = {};
}
