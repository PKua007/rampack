//
// Created by Piotr Kubala on 05/05/2021.
//

#include <sstream>
#include <algorithm>
#include <numeric>
#include <utility>
#include <ZipIterator.hpp>

#include "SmecticOrder.h"


auto SmecticOrder::calculateTau(const std::array<int, 3> &nTau_, const Packing &packing,
                                const std::vector<Vector<3>> &focalPoints, const std::vector<double> &functionValues)
{
    Expects(focalPoints.size() == functionValues.size());

    auto dimInv = packing.getBox().getDimensions().inverse();
    Vector<3> kTauVector_;
    for (std::size_t i{}; i < 3; i++)
        kTauVector_[i] = 2 * M_PI * nTau_[i];
    kTauVector_ = dimInv.transpose() * kTauVector_;
    double volume = packing.getVolume();

    using namespace std::complex_literals;
    std::complex<double> tau_{};
    for (const auto &[focalPoint_, functionValue] : Zip(focalPoints, functionValues))
        tau_ += functionValue * std::exp(1i * (kTauVector_ * focalPoint_));
    tau_ /= volume;

    return std::make_pair(tau_, kTauVector_);
}

void SmecticOrder::calculate(const Packing &packing, [[maybe_unused]] double temperature,
                             [[maybe_unused]] double pressure, [[maybe_unused]] const ShapeTraits &shapeTraits)
{
    auto focalPoints = packing.dumpNamedPoints(shapeTraits.getGeometry(), this->focalPoint);
    auto functionValues = this->calculateFunctionValues(packing, shapeTraits);

    this->tau = 0;
    this->nTau = {0, 0, 0};

    // Make sure that the first non-zero range coordinate i get iterated from 0, not from -nTauRanges[i]
    // This way we make sure that equivalent k and -k vectors aren't calculated twice
    for (int cx{}; cx <= this->nTauRanges[0]; cx++) {
        int cyBeg = (cx == 0 ? 0 : -this->nTauRanges[1]);
        for (int cy = cyBeg; cy <= this->nTauRanges[1]; cy++) {
            int czBeg = ((cx == 0 && cy == 0) ? 0 : -this->nTauRanges[2]);
            for (int cz = czBeg; cz <= this->nTauRanges[2]; cz++) {
                std::array<int, 3> nTau_{cx, cy, cz};
                if (nTau_ == std::array<int, 3>{0, 0, 0})
                    continue;

                auto [tau_, kTauVector_] = calculateTau(nTau_, packing, focalPoints, functionValues);
                if (std::abs(tau_) > std::abs(this->tau)) {
                    this->tau = tau_;
                    this->nTau = nTau_;
                    this->kTauVector = kTauVector_;
                }
            }
        }
    }
}

std::vector<std::string> SmecticOrder::getNominalValues() const {
    std::ostringstream ostr;
    ostr << this->nTau[0] << "." << this->nTau[1] << "." << this->nTau[2];
    return {ostr.str()};
}

SmecticOrder::SmecticOrder(const std::array<std::size_t, 3> &nTauRanges, bool dumpTauVector_, std::string focalPoint,
                           std::shared_ptr<ShapeFunction> shapeFunction)
        : dumpTauVector{dumpTauVector_}, focalPoint{std::move(focalPoint)}, shapeFunction{std::move(shapeFunction)}
{
    Expects(std::any_of(nTauRanges.begin(), nTauRanges.end(), [](int i) { return i != 0; }));
    Expects(std::all_of(nTauRanges.begin(), nTauRanges.end(), [](int i) { return i >= 0; }));
    std::copy(nTauRanges.begin(), nTauRanges.end(), this->nTauRanges.begin());
}

std::vector<std::string> SmecticOrder::getIntervalHeader() const {
    std::string functionName = this->shapeFunction->getName();
    std::string tauName = (functionName == "const") ? "tau" : ("tau_" + functionName);
    std::string kPrefix = tauName + "_k_";

    if (this->dumpTauVector)
        return {tauName, kPrefix + "x", kPrefix + "y", kPrefix + "z"};
    else
        return {tauName};
}

std::vector<double> SmecticOrder::getIntervalValues() const {
    if (this->dumpTauVector)
        return {std::abs(this->tau), this->kTauVector[0], this->kTauVector[1], this->kTauVector[2]};
    else
        return {std::abs(this->tau)};
}

std::vector<double> SmecticOrder::calculateFunctionValues(const Packing &packing, const ShapeTraits &traits) const {
    std::vector<double> values;
    values.reserve(packing.size());
    for (const auto &shape : packing)
        values.push_back(this->shapeFunction->calculate(shape, traits));
    return values;
}

std::vector<std::string> SmecticOrder::getNominalHeader() const {
    std::string functionName = this->shapeFunction->getName();
    if (functionName == "const")
        return {"tau_hkl"};
    else
        return {"tau_" + functionName + "_hkl"};
}

std::string SmecticOrder::getName() const {
    std::string functionName = this->shapeFunction->getName();
    if (functionName == "const")
        return "smectic order";
    else
        return functionName + " smectic order";
}
