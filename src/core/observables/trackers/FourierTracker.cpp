//
// Created by pkua on 20.10.22.
//

#include <algorithm>
#include <iterator>

#include "FourierTracker.h"
#include "utils/Assertions.h"
#include "core/PeriodicBoundaryConditions.h"


std::string FourierTracker::getModeName() const {
    return this->functionName + "_fourier";
}


void FourierTracker::calculateOrigin(const Packing &packing, const ShapeTraits &shapeTraits) {
    FourierValues fourierValues = this->calculateFourierValues(packing, shapeTraits);
    Vector<3> originPosRel = this->calculateRelativeOriginPos(fourierValues);
    originPosRel = this->normalizeOriginPos(originPosRel);
    this->originPos = packing.getBox().relativeToAbsolute(originPosRel);
}

Vector<3> FourierTracker::calculateRelativeOriginPos(const FourierTracker::FourierValues &fourierValues) const {
    switch (this->nonzeroIdxs.size()) {
        case 1:
            return this->calculateRelativeOriginPos1D(fourierValues);
        case 2:
            return this->calculateRelativeOriginPos2D(fourierValues);
        default:
            throw AssertionException("FourierTracker: zeros = " + std::to_string(this->nonzeroIdxs.size()));
    }
}

Vector<3> FourierTracker::calculateRelativeOriginPos1D(const FourierTracker::FourierValues &fourierValues) const {
    std::size_t nonzeroIdx = this->nonzeroIdxs.front();
    std::array<std::size_t, 3> idxs{};
    idxs[nonzeroIdx] = 0;
    double c = fourierValues[idxs[0]][idxs[1]][idxs[2]];
    idxs[nonzeroIdx] = 1;
    double d = fourierValues[idxs[0]][idxs[1]][idxs[2]];

    if (c*c + d*d < AMPLITUDE_EPSILON*AMPLITUDE_EPSILON)
        return {};

    auto wavenumber = static_cast<double>(this->wavenumbers[nonzeroIdx]);
    Vector<3> relPos{};
    relPos[nonzeroIdx] = std::fmod(std::atan2(d, c)/wavenumber/2/M_PI + 1, 1);
    return relPos;
}

Vector<3> FourierTracker::calculateRelativeOriginPos2D(const FourierTracker::FourierValues &fourierValues) const {
    std::size_t nonzeroIdx1 = this->nonzeroIdxs[0];
    std::size_t nonzeroIdx2 = this->nonzeroIdxs[1];
    std::array<std::size_t, 3> idxs{};
    idxs[nonzeroIdx1] = 0;
    idxs[nonzeroIdx2] = 0;
    double A = fourierValues[idxs[0]][idxs[1]][idxs[2]];
    idxs[nonzeroIdx1] = 0;
    idxs[nonzeroIdx2] = 1;
    double B = fourierValues[idxs[0]][idxs[1]][idxs[2]];
    idxs[nonzeroIdx1] = 1;
    idxs[nonzeroIdx2] = 0;
    double C = fourierValues[idxs[0]][idxs[1]][idxs[2]];
    idxs[nonzeroIdx1] = 1;
    idxs[nonzeroIdx2] = 1;
    double D = fourierValues[idxs[0]][idxs[1]][idxs[2]];

    if (A*A + B*B + C*C + D*D < AMPLITUDE_EPSILON*AMPLITUDE_EPSILON)
        return {};

    double a = std::atan2(C + B, A - D);
    double b = std::atan2(C - B, A + D);
    auto wavenumber1 = static_cast<double>(this->wavenumbers[nonzeroIdx1]);
    auto wavenumber2 = static_cast<double>(this->wavenumbers[nonzeroIdx2]);
    Vector<3> relPos{};
    relPos[nonzeroIdx1] = std::fmod(((a + b)/2)/wavenumber1/2/M_PI + 1, 1);
    relPos[nonzeroIdx2] = std::fmod(((a - b)/2)/wavenumber2/2/M_PI + 1, 1);
    return relPos;
}

FourierTracker::FourierValues FourierTracker::calculateFourierValues(const Packing &packing,
                                                                     const ShapeTraits &shapeTraits) const
{
    FourierValues fourierValues{};

    for (std::size_t i{}; i < this->fourierFunctions[0].size(); i++) {
        for (std::size_t j{}; j < this->fourierFunctions[1].size(); j++) {
            for (std::size_t k{}; k < this->fourierFunctions[2].size(); k++) {
                const auto &f0 = this->fourierFunctions[0][i];
                const auto &f1 = this->fourierFunctions[1][j];
                const auto &f2 = this->fourierFunctions[2][k];
                auto &fourierValue = fourierValues[i][j][k];

                auto fProduct = [&f0, &f1, &f2](const Vector<3> &pos) {
                    return f0(pos[0]) * f1(pos[1]) * f2(pos[2]);
                };

                fourierValue = 0;
                for (std::size_t shapeI{}; shapeI < packing.size(); shapeI++) {
                    const auto &shape = packing[shapeI];
                    double functionMapping = this->function(shape, shapeTraits);
                    Vector<3> relativePos = packing.getBox().absoluteToRelative(shape.getPosition());
                    fourierValue += fProduct(relativePos) * functionMapping;
                }
            }
        }
    }

    return fourierValues;
}

FourierTracker::FourierTracker(const std::array<std::size_t, 3> &wavenumbers, Function function,
                               std::string functionName)
        : wavenumbers{wavenumbers}, function{std::move(function)}, functionName{std::move(functionName)}
{
    for (std::size_t i{}; i < 3; i++) {
        std::size_t wavenumber = this->wavenumbers[i];
        Expects(wavenumber >= 0);
        if (wavenumber > 0)
            this->nonzeroIdxs.push_back(wavenumber);
    }
    Expects(!this->nonzeroIdxs.empty());
    ExpectsMsg(this->nonzeroIdxs.size() != 3, "3D FourierTracker is not implemented yet");

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
    if (!this->previousRelValue.has_value()) {
        this->previousRelValue = originPosRel;
        return originPosRel;
    }

    PeriodicBoundaryConditions pbc(1);
    Vector<3> bestOriginPosRel = originPosRel;
    double deltaR2 = pbc.getDistance2(*this->previousRelValue, originPosRel);
    std::array<std::size_t, 3> idxs{};
    for (idxs[0] = 0; idxs[0] < std::max(2*this->wavenumbers[0], 1ul); idxs[0]++) {
        for (idxs[1] = 0; idxs[1] < std::max(2*this->wavenumbers[1], 1ul); idxs[1]++) {
            for (idxs[2] = 0; idxs[2] < std::max(2*this->wavenumbers[2], 1ul); idxs[2]++) {
                if (std::accumulate(idxs.begin(), idxs.end(), 0ul) % 2 != 0)
                    continue;

                Vector<3> nextOriginPosRel = originPosRel;
                for (std::size_t i{}; i < 3; i++)
                    if (this->wavenumbers[idxs[i]] > 0)
                        nextOriginPosRel[idxs[i]] += static_cast<double>(idxs[i])/static_cast<double>(2*this->wavenumbers[idxs[i]]);

                double newDeltaR2 = pbc.getDistance2(*this->previousRelValue, nextOriginPosRel);
                if (newDeltaR2 < deltaR2) {
                    deltaR2 = newDeltaR2;
                    bestOriginPosRel = nextOriginPosRel;
                }
            }
        }
    }

    this->previousRelValue = bestOriginPosRel;
    return bestOriginPosRel;
}
