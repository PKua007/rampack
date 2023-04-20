//
// Created by Piotr Kubala on 20/04/2023.
//

#include "ProbabilityEvolution.h"


ProbabilityEvolution::ProbabilityEvolution(double maxDistance, std::pair<double, double> variableRange,
                                           std::size_t numDistanceBins, std::size_t numVariableBins,
                                           std::shared_ptr<PairEnumerator> pairEnumerator,
                                           std::shared_ptr<CorrelationFunction> function, std::size_t numThreads)
        : PairConsumer(numThreads), maxDistance{maxDistance}, variableRange{variableRange},
          histogramBuilder({0, variableRange.first}, {maxDistance, variableRange.second},
                           {numDistanceBins, numVariableBins}, numThreads),
          pairEnumerator{std::move(pairEnumerator)}, function{std::move(function)}
{
    Expects(maxDistance > 0);
}

void ProbabilityEvolution::addSnapshot(const Packing &packing, [[maybe_unused]] double temperature,
                                       [[maybe_unused]] double pressure, const ShapeTraits &shapeTraits)
{
    this->pairEnumerator->enumeratePairs(packing, shapeTraits, *this);
    this->histogramBuilder.nextSnapshot();
}

void ProbabilityEvolution::print(std::ostream &out) const {
    for (auto [xy, z] : this->histogramBuilder.dumpValues(HistogramBuilder<2>::ReductionMethod::SUM))
        out << xy[0] << " " << xy[1] << " " << z << std::endl;
}

void ProbabilityEvolution::clear() {
    this->histogramBuilder.clear();
}

std::string ProbabilityEvolution::getSignatureName() const {
    return "prob_" + this->function->getSignatureName() + "_" + this->pairEnumerator->getSignatureName();
}

void ProbabilityEvolution::consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                                       double distance, const ShapeTraits &shapeTraits)
{
    if (idxPair.first == idxPair.second)
        return;

    if (distance > this->maxDistance)
        return;

    const auto &shape1 = packing[idxPair.first];
    const auto &shape2 = packing[idxPair.second];
    double value = this->function->calculate(shape1, shape2, shapeTraits);
    if (value < this->variableRange.first || value > this->variableRange.second)
        return;

    this->histogramBuilder.add({distance, value}, 1);
}
