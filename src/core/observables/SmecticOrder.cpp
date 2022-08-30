//
// Created by Piotr Kubala on 05/05/2021.
//

#include <sstream>
#include <algorithm>
#include <numeric>
#include <utility>

#include "SmecticOrder.h"


auto SmecticOrder::calculateTau(const std::array<int, 3> &kTau_, const Packing &packing, const std::vector<Vector<3>> &focalPoints)
{
    auto dimInv = packing.getBox().getDimensions().inverse();
    Vector<3> kTauVector_;
    for (std::size_t i{}; i < 3; i++)
        kTauVector_[i] = 2 * M_PI * kTau_[i];
    kTauVector_ = dimInv.transpose() * kTauVector_;
    double volume = packing.getVolume();

    auto tauAccumulator = [kTauVector_](std::complex<double> tau_, const Vector<3> &focalPoint_) {
        using namespace std::complex_literals;
        return tau_ + std::exp(1i * (kTauVector_ * focalPoint_));
    };
    auto tau_= std::accumulate(focalPoints.begin(), focalPoints.end(), std::complex<double>{0}, tauAccumulator);
    tau_ /= volume;

    return std::make_pair(tau_, kTauVector_);
}

void SmecticOrder::calculate(const Packing &packing, [[maybe_unused]] double temperature,
                             [[maybe_unused]] double pressure, [[maybe_unused]] const ShapeTraits &shapeTraits)
{
    auto focalPoints = packing.dumpNamedPoints(shapeTraits.getGeometry(), this->focalPoint);

    this->tau = 0;
    this->kTau = {0, 0, 0};

    for (int cx{}; cx <= this->kTauRanges[0]; cx++) {
        int cyBeg = (cx == 0 ? 0 : -this->kTauRanges[1]);
        for (int cy = cyBeg; cy <= this->kTauRanges[1]; cy++) {
            int czBeg = ((cx == 0 && cy == 0) ? 1 : -this->kTauRanges[2]);
            for (int cz = czBeg; cz < this->kTauRanges[2]; cz++) {
                std::array<int, 3> kTau_{cx, cy, cz};
                auto [tau_, kTauVector_] = calculateTau(kTau_, packing, focalPoints);
                if (std::abs(tau_) > std::abs(this->tau)) {
                    this->tau = tau_;
                    this->kTau = kTau_;
                    this->kTauVector = kTauVector_;
                }
            }
        }
    }
}

std::vector<std::string> SmecticOrder::getNominalValues() const {
    std::ostringstream ostr;
    ostr << this->kTau[0] << "." << this->kTau[1] << "." << this->kTau[2];
    return {ostr.str()};
}

SmecticOrder::SmecticOrder(const std::array<int, 3> &kTauRanges, bool dumpTauVector_, std::string focalPoint)
        : dumpTauVector{dumpTauVector_}, focalPoint{std::move(focalPoint)}, kTauRanges{kTauRanges}
{
    Expects(std::any_of(kTauRanges.begin(), kTauRanges.end(), [](int i) { return i != 0; }));
    Expects(std::all_of(kTauRanges.begin(), kTauRanges.end(), [](int i) { return i >= 0; }));
}

std::vector<std::string> SmecticOrder::getIntervalHeader() const {
    if (this->dumpTauVector)
        return {"tau", "k_x", "k_y", "k_z"};
    else
        return {"tau"};
}

std::vector<double> SmecticOrder::getIntervalValues() const {
    if (this->dumpTauVector)
        return {std::abs(this->tau), this->kTauVector[0], this->kTauVector[1], this->kTauVector[2]};
    else
        return {std::abs(this->tau)};
}
