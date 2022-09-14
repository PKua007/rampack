//
// Created by pkua on 12.09.22.
//

#include <algorithm>
#include <complex>
#include <utility>

#include "LayerwiseRadialEnumerator.h"
#include "utils/Assertions.h"


void LayerwiseRadialEnumerator::enumeratePairs(const Packing &packing, const ShapeTraits &shapeTraits,
                                               PairConsumer &pairConsumer) const
{
    auto dimInv = packing.getBox().getDimensions().inverse();
    Vector<3> kVector = 2*M_PI*(dimInv.transpose() * this->millerIndices);

    double layerDistance = 2*M_PI/kVector.norm();
    double areaDensity = packing.getNumberDensity() * layerDistance;

    auto focalPoints = packing.dumpNamedPoints(shapeTraits.getGeometry(), this->focalPoint);
    auto tauAccumulator = [kVector](std::complex<double> tau_, const Vector<3> &point) {
        using namespace std::complex_literals;
        return tau_ + std::exp(1i * (kVector * point));
    };
    auto tau = std::accumulate(focalPoints.begin(), focalPoints.end(), std::complex<double>{0}, tauAccumulator);
    auto tauAngle = std::arg(tau);

    const auto &bc = packing.getBoundaryConditions();
    for (std::size_t i{}; i < packing.size(); i++) {
        for (std::size_t j = i; j < packing.size(); j++) {
            const auto &pos1 = focalPoints[i];
            auto pos2 = focalPoints[j];
            pos2 += bc.getTranslation(pos1, pos2);

            double layer1 = std::round((pos1 * kVector - tauAngle) / (2 * M_PI));
            double layer2 = std::round((pos2 * kVector - tauAngle) / (2 * M_PI));
            if (layer1 != layer2)
                continue;

            double distance = (pos2 - pos1).norm();
            double jacobian = 2 * M_PI * distance * areaDensity;
            pairConsumer.consumePair(packing, {i, j}, distance, jacobian);
        }
    }
}

LayerwiseRadialEnumerator::LayerwiseRadialEnumerator(const std::array<std::size_t, 3> &millerIndices,
                                                     std::string focalPoint)
        : focalPoint{std::move(focalPoint)}
{
    Expects(std::any_of(millerIndices.begin(), millerIndices.end(), [](auto i) { return i != 0; }));
    std::copy(millerIndices.begin(), millerIndices.end(), this->millerIndices.begin());
}
