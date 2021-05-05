//
// Created by Piotr Kubala on 05/05/2021.
//

#include <sstream>
#include <algorithm>
#include <numeric>

#include "SmecticOrder.h"

void SmecticOrder::calculate(const Packing &packing, [[maybe_unused]] double temperature,
                             [[maybe_unused]] double pressure, [[maybe_unused]] const ShapeTraits &shapeTraits)
{
    this->tau = 0;
    this->kTau = {0, 0, 0};

    for (int cx{}; cx <= this->kTauRanges[0]; cx++) {
        int cyBeg = (cx == 0 ? 0 : -this->kTauRanges[1]);
        for (int cy = cyBeg; cy <= this->kTauRanges[1]; cy++) {
            int czBeg = ((cx == 0 && cy == 0) ? 1 : -this->kTauRanges[2]);
            for (int cz = czBeg; cz < this->kTauRanges[2]; cz++) {
                std::array<int, 3> kTau_{cx, cy, cz};
                std::complex<double> tau_ = calculateTau(kTau_, packing);
                if (std::abs(tau_) > std::abs(this->tau)) {
                    this->tau = tau_;
                    this->kTau = kTau_;
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

SmecticOrder::SmecticOrder(const std::array<int, 3> &kTauRanges) : kTauRanges{kTauRanges} {
    Expects(std::any_of(kTauRanges.begin(), kTauRanges.end(), [](int i) { return i != 0; }));
    Expects(std::all_of(kTauRanges.begin(), kTauRanges.end(), [](int i) { return i >= 0; }));
}

std::complex<double> SmecticOrder::calculateTau(const std::array<int, 3> &kTau_, const Packing &packing) {
    auto dim = packing.getDimensions();
    Vector<3> wavevector;
    for (std::size_t i{}; i < 3; i++)
        wavevector[i] = 2*M_PI*kTau_[i]/dim[i];
    double volume = packing.getVolume();

    auto tauAccumulator = [wavevector](std::complex<double> tau_, const Shape &shape) {
        using namespace std::complex_literals;
        return tau_ + std::exp(1i * (wavevector * shape.getPosition()));
    };
    return std::accumulate(packing.begin(), packing.end(), std::complex<double>{0}, tauAccumulator) / volume;
}
