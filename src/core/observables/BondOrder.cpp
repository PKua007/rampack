//
// Created by pkua on 05.04.2022.
//

#include <algorithm>
#include <iterator>
#include <complex>

#include "BondOrder.h"


BondOrder::BondOrder(std::vector<std::size_t> ranks, const std::array<int, 3> &layerWavenumber)
        : ranks{std::move(ranks)}, layerWavenumber{layerWavenumber}
{
    Expects(!this->ranks.empty());
    Expects(std::all_of(this->ranks.begin(), this->ranks.end(), [](auto rank) { return rank >= 2; }));
    Expects(std::any_of(this->layerWavenumber.begin(), this->layerWavenumber.end(), [](auto n) { return n != 0; }));

    std::sort(this->ranks.begin(), this->ranks.end());
    this->psis.resize(this->ranks.size());
    this->header.reserve(this->ranks.size());
    std::transform(this->ranks.begin(), this->ranks.end(), std::back_inserter(this->header),
                   [](auto rank) { return "psi" + std::to_string(rank); });
}

auto BondOrder::findPlaneVectors(const Packing &packing) {
    auto dimInv = packing.getBox().getDimensions().inverse();
    Vector<3> normalVector;
    for (std::size_t i{}; i < 3; i++)
        normalVector[i] = 2 * M_PI * static_cast<double>(this->layerWavenumber[i]);
    normalVector = (dimInv.transpose() * normalVector).normalized();

    std::size_t minIdx = std::min_element(normalVector.begin(), normalVector.end()) - normalVector.begin();
    Vector<3> nonParallel;
    nonParallel[minIdx] = 1;

    Vector<3> planeVector1 = (normalVector ^ nonParallel).normalized();
    Vector<3> planeVector2 = normalVector ^ planeVector1;

    return std::make_pair(planeVector1, planeVector2);
}

void BondOrder::calculate(const Packing &packing, [[maybe_unused]] double temperature, [[maybe_unused]] double pressure,
                          [[maybe_unused]] const ShapeTraits &shapeTraits)
{
    std::vector<KnnVector> knn = this->constructKnn(packing);
    auto [planeVector1, planeVector2] = this->findPlaneVectors(packing);
    for (std::size_t i{}; i < this->ranks.size(); i++)
        this->psis[i] = BondOrder::doCalculateBondOrder(packing, this->ranks[i], knn, planeVector1, planeVector2);
}

double BondOrder::doCalculateBondOrder(const Packing &packing, std::size_t rank, const std::vector<KnnVector> &knn,
                                       const Vector<3> &planeVector1, const Vector<3> &planeVector2)
{
    std::complex<double> psiComplex{};
    for (std::size_t particleIdx{}; particleIdx < packing.size(); particleIdx++) {
        const auto &particlePos = packing[particleIdx].getPosition();
        const auto &neighboursIdxs = knn[particleIdx];
        for (std::size_t neighbourIdx{}; neighbourIdx < rank; neighbourIdx++) {
            auto neighbourParticleIdx = neighboursIdxs[neighbourIdx].first;
            const auto &neighbourPos = packing[neighbourParticleIdx].getPosition();
            const BoundaryConditions &bc = packing.getBoundaryConditions();
            Vector<3> diff = neighbourPos - particlePos + bc.getTranslation(particlePos, neighbourPos);

            double coord1 = planeVector1 * diff;
            double coord2 = planeVector2 * diff;
            double angle = atan2(coord2, coord1);

            using namespace std::complex_literals;
            psiComplex += std::exp(1i * static_cast<double>(rank) * angle);
        }
    }

    return std::abs(psiComplex) / static_cast<double>(rank * packing.size());
}

std::vector<BondOrder::KnnVector> BondOrder::constructKnn(const Packing &packing) {
    std::size_t highestRank = this->ranks.back();
    KnnVector knnInitalEntry(highestRank, {0, std::numeric_limits<double>::infinity()});
    std::vector<KnnVector> knn(packing.size(), knnInitalEntry);
    for (std::size_t i{}; i < packing.size(); i++) {
        for (std::size_t j = i + 1; j < packing.size(); j++) {
            const auto &shape1 = packing[i];
            const auto &shape2 = packing[j];
            const auto &bc = packing.getBoundaryConditions();
            double distance2 = bc.getDistance2(shape1.getPosition(), shape2.getPosition());

            insertDistance(knn[i], j, distance2);
            insertDistance(knn[j], i, distance2);
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
