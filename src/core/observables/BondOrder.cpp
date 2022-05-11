//
// Created by pkua on 05.04.2022.
//

#include <algorithm>
#include <iterator>
#include <complex>

#include "BondOrder.h"


BondOrder::BondOrder(std::vector<std::size_t> ranks, const std::array<int, 3> &planeMillerIndices)
        : ranks{std::move(ranks)}
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

void BondOrder::calculateLayerGeometry(const Packing &packing) {
    auto dimInv = packing.getBox().getDimensions().inverse();
    this->kVector = 2*M_PI*(dimInv.transpose() * this->millerIndices);

    std::size_t minIdx = std::min_element(this->kVector.begin(), this->kVector.end()) - this->kVector.begin();
    Vector<3> nonParallel;
    nonParallel[minIdx] = 1;

    this->planeVector1 = (this->kVector ^ nonParallel).normalized();
    this->planeVector2 = (this->kVector ^ planeVector1).normalized();

    auto tauAccumulator = [this](std::complex<double> tau_, const Shape &shape) {
        using namespace std::complex_literals;
        return tau_ + std::exp(1i * (this->kVector * shape.getPosition()));
    };
    auto tau = std::accumulate(packing.begin(), packing.end(), std::complex<double>{0}, tauAccumulator);
    this->tauAngle = std::arg(tau);
}

void BondOrder::calculate(const Packing &packing, [[maybe_unused]] double temperature, [[maybe_unused]] double pressure,
                          [[maybe_unused]] const ShapeTraits &shapeTraits)
{
    this->calculateLayerGeometry(packing);
    std::vector<KnnVector> knn = this->constructKnn(packing);
    for (std::size_t i{}; i < this->ranks.size(); i++)
        this->psis[i] = BondOrder::doCalculateBondOrder(packing, this->ranks[i], knn, planeVector1, planeVector2);
}

double BondOrder::doCalculateBondOrder(const Packing &packing, std::size_t rank, const std::vector<KnnVector> &knn,
                                       const Vector<3> &planeVector1, const Vector<3> &planeVector2)
{
    double psi{};
    for (std::size_t particleIdx{}; particleIdx < packing.size(); particleIdx++) {
        const auto &particlePos = packing[particleIdx].getPosition();
        const auto &neighboursIdxs = knn[particleIdx];
        // Skip particles which have too few neighbours - effectively they have psi = 0
        if (neighboursIdxs.size() < rank)
            continue;

        std::complex<double> localPsi{};
        for (std::size_t neighbourIdx{}; neighbourIdx < rank; neighbourIdx++) {
            auto neighbourParticleIdx = neighboursIdxs[neighbourIdx].first;
            const auto &neighbourPos = packing[neighbourParticleIdx].getPosition();
            const BoundaryConditions &bc = packing.getBoundaryConditions();
            Vector<3> diff = neighbourPos - particlePos + bc.getTranslation(particlePos, neighbourPos);

            double coord1 = planeVector1 * diff;
            double coord2 = planeVector2 * diff;
            double angle = atan2(coord2, coord1);

            using namespace std::complex_literals;
            localPsi += std::exp(1i * static_cast<double>(rank) * angle);
        }
        psi += std::abs(localPsi) / static_cast<double>(rank);
    }

    return std::abs(psi) / static_cast<double>(packing.size());
}

std::vector<BondOrder::KnnVector> BondOrder::constructKnn(const Packing &packing) {
    std::size_t highestRank = this->ranks.back();
    KnnVector knnInitalEntry(highestRank, {0, std::numeric_limits<double>::infinity()});
    std::vector<KnnVector> knn(packing.size(), knnInitalEntry);
    Vector<3> kVectorNormalized = this->kVector.normalized();
    for (std::size_t i{}; i < packing.size(); i++) {
        for (std::size_t j = i + 1; j < packing.size(); j++) {
            const auto &pos1 = packing[i].getPosition();
            auto pos2 = packing[j].getPosition();
            const auto &bc = packing.getBoundaryConditions();
            pos2 += bc.getTranslation(pos1, pos2);

            double layer1 = std::round((pos1 * this->kVector - this->tauAngle) / (2 * M_PI));
            double layer2 = std::round((pos2 * this->kVector - this->tauAngle) / (2 * M_PI));
            if (layer1 != layer2)
                continue;

            Vector<3> diff = pos2 - pos1;
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
