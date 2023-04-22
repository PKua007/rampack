//
// Created by Piotr Kubala on 20/04/2023.
//

#ifndef RAMPACK_PROBABILITYEVOLUTION_H
#define RAMPACK_PROBABILITYEVOLUTION_H

#include "HistogramBuilder.h"
#include "core/BulkObservable.h"
#include "correlation/PairConsumer.h"
#include "correlation/PairEnumerator.h"
#include "CorrelationFunction.h"


class ProbabilityEvolution : public BulkObservable, public PairConsumer {
public:
    enum class Normalization {
        NONE,
        PDF,
        UNIT
    };

private:
    double maxDistance{};
    std::pair<double, double> variableRange{};
    HistogramBuilder<2> histogramBuilder;
    std::shared_ptr<PairEnumerator> pairEnumerator;
    std::shared_ptr<CorrelationFunction> function;
    Normalization normalization{};

    void renormalizeHistogram(Histogram2D &histogram) const;

public:
    ProbabilityEvolution(double maxDistance, std::pair<double, double> variableRange, std::size_t numDistanceBins,
                         std::size_t numVariableBins, std::shared_ptr<PairEnumerator> pairEnumerator,
                         std::shared_ptr<CorrelationFunction> function,
                         Normalization normalization = Normalization::PDF, std::size_t numThreads = 1);

    void addSnapshot(const Packing &packing, double temperature, double pressure,
                     const ShapeTraits &shapeTraits) override;
    void print(std::ostream &out) const override;
    void clear() override;
    [[nodiscard]] std::string getSignatureName() const override;
    void consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair, double distance,
                     const ShapeTraits &shapeTraits) override;
};


#endif //RAMPACK_PROBABILITYEVOLUTION_H
