//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_PAIRAVERAGEDCORRELATION_H
#define RAMPACK_PAIRAVERAGEDCORRELATION_H

#include <memory>

#include "core/BulkObservable.h"
#include "PairConsumer.h"
#include "PairEnumerator.h"
#include "Histogram.h"
#include "CorrelationFunction.h"


class PairAveragedCorrelation : public BulkObservable, public PairConsumer {
private:
    std::unique_ptr<PairEnumerator> pairEnumerator;
    std::unique_ptr<CorrelationFunction> correlationFunction;
    Histogram histogram;
    const ShapeTraits *lastTraits{};

public:
    PairAveragedCorrelation(std::unique_ptr<PairEnumerator> pairEnumerator,
                            std::unique_ptr<CorrelationFunction> correlationFunction, double maxR, std::size_t numBins)
            : pairEnumerator{std::move(pairEnumerator)}, correlationFunction{std::move(correlationFunction)},
              histogram(0, maxR, numBins)
    { }

    void addSnapshot(const Packing &packing, double temperature, double pressure,
                     const ShapeTraits &shapeTraits) override;
    void print(std::ostream &out) const override;
    void clear() override { this->histogram.clear(); }
    void consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                     double distance) override;

    [[nodiscard]] std::string getSignatureName() const override {
        return this->correlationFunction->getSignatureName() + "_" + this->pairEnumerator->getSignatureName();
    }
};


#endif //RAMPACK_PAIRAVERAGEDCORRELATION_H
