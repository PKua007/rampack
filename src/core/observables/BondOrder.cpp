//
// Created by pkua on 05.04.2022.
//

#include <algorithm>
#include <iterator>
#include <complex>
#include <utility>

#include "BondOrder.h"


BondOrder::BondOrder(std::vector<std::size_t> ranks, const std::array<int, 3> &planeMillerIndices,
                     std::string layeringPointName, std::string bondOrderPointName)
        : ranks{std::move(ranks)}, layeringPointName{std::move(layeringPointName)},
          bondOrderPointName{std::move(bondOrderPointName)}
{
    Expects(!this->ranks.empty());
    Expects(std::all_of(this->ranks.begin(), this->ranks.end(), [](auto rank) { return rank >= 2; }));
    Expects(std::any_of(planeMillerIndices.begin(), planeMillerIndices.end(), [](auto n) { return n != 0; }));

    std::copy(planeMillerIndices.begin(), planeMillerIndices.end(), this->millerIndices.begin());
    std::sort(this->ranks.begin(), this->ranks.end());
    this->psis.resize(this->ranks.size());
    this->header.reserve(this->ranks.size());
    std::transform(this->ranks.begin(), this->ranks.end(), std::back_inserter(this->header),
                   [](auto rank) { return "psi" + std::to_string(rank); });
}

void BondOrder::calculateLayerGeometry(const Packing &packing, const std::vector<Vector<3>> &layeringPoints) {
    auto dimInv = packing.getBox().getDimensions().inverse();
    this->kVector = 2*M_PI*(dimInv.transpose() * this->millerIndices);

    std::size_t minIdx = std::min_element(this->kVector.begin(), this->kVector.end()) - this->kVector.begin();
    Vector<3> nonParallel;
    nonParallel[minIdx] = 1;

    this->planeVector1 = (this->kVector ^ nonParallel).normalized();
    this->planeVector2 = (this->kVector ^ this->planeVector1).normalized();

    auto tauAccumulator = [this](std::complex<double> tau_, const Vector<3> &point) {
        using namespace std::complex_literals;
        return tau_ + std::exp(1i * (this->kVector * point));
    };
    auto tau = std::accumulate(layeringPoints.begin(), layeringPoints.end(), std::complex<double>{0}, tauAccumulator);
    this->tauAngle = std::arg(tau);
}

void BondOrder::calculate(const Packing &packing, [[maybe_unused]] double temperature, [[maybe_unused]] double pressure,
                          [[maybe_unused]] const ShapeTraits &shapeTraits)
{
    auto layeringPoints = packing.dumpNamedPoints(shapeTraits.getGeometry(), this->layeringPointName);
    auto bondOrderPoints = packing.dumpNamedPoints(shapeTraits.getGeometry(), this->bondOrderPointName);

    this->calculateLayerGeometry(packing, layeringPoints);
    const auto &bc = packing.getBoundaryConditions();
    std::vector<KnnVector> knn = this->constructKnn(layeringPoints, bondOrderPoints, bc);
    for (std::size_t i{}; i < this->ranks.size(); i++)
        this->psis[i] = BondOrder::doCalculateBondOrder(bondOrderPoints, this->ranks[i], knn, bc);
}

double BondOrder::doCalculateBondOrder(const std::vector<Vector<3>> &bondOrderPoints, std::size_t rank,
                                       const std::vector<KnnVector> &knn, const BoundaryConditions &bc)
{
    double psi{};
    for (std::size_t particleIdx{}; particleIdx < bondOrderPoints.size(); particleIdx++) {
        const auto &particlePos = bondOrderPoints[particleIdx];
        const auto &neighboursIdxs = knn[particleIdx];
        // Skip particles which have too few neighbours - effectively they have psi = 0
        if (neighboursIdxs.size() < rank)
            continue;

        std::complex<double> localPsi{};
        for (std::size_t neighbourIdx{}; neighbourIdx < rank; neighbourIdx++) {
            auto neighbourParticleIdx = neighboursIdxs[neighbourIdx].first;
            const auto &neighbourPos = bondOrderPoints[neighbourParticleIdx];
            Vector<3> diff = neighbourPos - particlePos + bc.getTranslation(particlePos, neighbourPos);

            double coord1 = this->planeVector1 * diff;
            double coord2 = this->planeVector2 * diff;
            double angle = atan2(coord2, coord1);

            using namespace std::complex_literals;
            localPsi += std::exp(1i * static_cast<double>(rank) * angle);
        }
        psi += std::abs(localPsi) / static_cast<double>(rank);
    }

    return std::abs(psi) / static_cast<double>(bondOrderPoints.size());
}

std::vector<BondOrder::KnnVector> BondOrder::constructKnn(const std::vector<Vector<3>> &layeringPoints,
                                                          const std::vector<Vector<3>> &bondOrderPoints,
                                                          const BoundaryConditions &bc)
{
    Expects(layeringPoints.size() == bondOrderPoints.size());
    std::size_t size = layeringPoints.size();

    std::size_t highestRank = this->ranks.back();
    KnnVector knnInitalEntry(highestRank, {0, std::numeric_limits<double>::infinity()});
    std::vector<KnnVector> knn(size, knnInitalEntry);
    Vector<3> kVectorNormalized = this->kVector.normalized();
    for (std::size_t i{}; i < size; i++) {
        for (std::size_t j = i + 1; j < size; j++) {
            const auto &layerPos1 = layeringPoints[i];
            auto layerPos2 = layeringPoints[j];
            layerPos2 += bc.getTranslation(layerPos1, layerPos2);

            double layer1 = std::round((layerPos1 * this->kVector - this->tauAngle) / (2 * M_PI));
            double layer2 = std::round((layerPos2 * this->kVector - this->tauAngle) / (2 * M_PI));
            if (layer1 != layer2)
                continue;

            const auto &bondOrderPos1 = bondOrderPoints[i];
            auto bondOrderPos2 = bondOrderPoints[j];
            bondOrderPos2 += bc.getTranslation(bondOrderPos1, bondOrderPos2);
            Vector<3> diff = bondOrderPos2 - bondOrderPos1;
            double distance2 = diff.norm2() - std::pow(diff * kVectorNormalized, 2);   // Subtract normal component
            distance2 = std::max(0.0, distance2);   // Make sure it is not < 0 due to numerical precision

            BondOrder::insertDistance(knn[i], j, distance2);
            BondOrder::insertDistance(knn[j], i, distance2);
        }
    }

    return knn;
}

void BondOrder::insertDistance(KnnVector &knnVector, std::size_t particleIdx, double distance2) {
    if (knnVector.back().second < distance2)
        return;

    knnVector.back().first = particleIdx;
    knnVector.back().second = distance2;

    for (std::size_t i = knnVector.size() - 1; i > 0; i--) {
        if (knnVector[i].second < knnVector[i - 1].second)
            std::swap(knnVector[i], knnVector[i - 1]);
        else
            break;
    }
}
