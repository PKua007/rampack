//
// Created by Piotr Kubala on 20/04/2023.
//

#include "ProbabilityEvolution.h"


ProbabilityEvolution::ProbabilityEvolution(double maxDistance, std::size_t numDistanceBins,
                                           std::shared_ptr<PairEnumerator> pairEnumerator,
                                           std::pair<double, double> functionRange,
                                           std::size_t numFunctionBins, std::shared_ptr<CorrelationFunction> function,
                                           Normalization normalization, bool printCount, std::size_t numThreads)
        : PairConsumer(numThreads), maxDistance{maxDistance}, functionRange{functionRange},
          histogramBuilder({0, functionRange.first}, {maxDistance, functionRange.second},
                           {numDistanceBins, numFunctionBins}, numThreads),
          pairEnumerator{std::move(pairEnumerator)}, function{std::move(function)}, normalization{normalization},
          printCount{printCount}
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
    Histogram2D histogram = this->histogramBuilder.dumpHistogram(ReductionMethod::SUM);
    this->renormalizeHistogram(histogram);

    for (auto [xy, z, count] : histogram.dumpValues()) {
        out << xy[0] << " " << xy[1] << " " << z;
        if (this->printCount)
            out << " " << count;
        out << std::endl;
    }
}

void ProbabilityEvolution::clear() {
    this->histogramBuilder.clear();
}

std::string ProbabilityEvolution::getSignatureName() const {
    return "prob_" + this->function->getSignatureName() + "_" + this->pairEnumerator->getSignatureName();
}

void ProbabilityEvolution::consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                                       const Vector<3> &distanceVector, const ShapeTraits &shapeTraits)
{
    if (idxPair.first == idxPair.second)
        return;

    double distance = distanceVector.norm();
    if (distance > this->maxDistance)
        return;

    const auto &shape1 = packing[idxPair.first];
    const auto &shape2 = packing[idxPair.second];
    double value = this->function->calculate(shape1, shape2, distanceVector, shapeTraits);
    if (value < this->functionRange.first || value > this->functionRange.second)
        return;

    this->histogramBuilder.add({distance, value}, 1);
}

void ProbabilityEvolution::renormalizeHistogram(Histogram2D &histogram) const {
    if (this->normalization == Normalization::AVG_COUNT)
        return;

    // Normalize by count so that all bins for a given distance sum to 1
    for (std::size_t distI{}; distI < histogram.getNumBins(0); distI++) {
        double distTotal{};
        for (std::size_t varI{}; varI < histogram.getNumBins(1); varI++)
            distTotal += histogram.atIndex({distI, varI}).value;

        if (distTotal == 0)
            continue;
        for (std::size_t funI{}; funI < histogram.getNumBins(1); funI++)
            histogram.atIndex({distI, funI}) /= distTotal;
    }

    // Apply the actual normalization
    switch (this->normalization) {
        case Normalization::PDF:
            histogram *= static_cast<double>(histogram.getNumBins(1));
            histogram /= (histogram.getMax(1) - histogram.getMin(1));
            break;
        case Normalization::UNIT:
            histogram *= static_cast<double>(histogram.getNumBins(1));
            break;
        case Normalization::AVG_COUNT:
            AssertThrow("Unreachable");
    }
}
