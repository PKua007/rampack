//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_PAIRBINNEDCORRELATION_H
#define RAMPACK_PAIRBINNEDCORRELATION_H

#include <memory>

#include "core/BulkObservable.h"
#include "PairConsumer.h"
#include "PairEnumerator.h"
#include "Histogram.h"
#include "CorrelationFunction.h"


class PairBinnedCorrelation : public BulkObservable, public PairConsumer {
private:
    std::unique_ptr<PairEnumerator> pairEnumerator;
    std::unique_ptr<CorrelationFunction> correlationFunction;
    Histogram histogram;

public:
    PairBinnedCorrelation(std::unique_ptr<PairEnumerator> pairEnumerator,
                          std::unique_ptr<CorrelationFunction> correlationFunction)
            : pairEnumerator{std::move(pairEnumerator)}, correlationFunction{std::move(correlationFunction)}
    { }

    void addSnapshot(const Packing &packing, double temperature, double pressure,
                     const ShapeTraits &shapeTraits) override;
    void print(std::ostream &out) const override;
    void clear() override { this->histogram.clear(); }
    void consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair, double distance,
                     double jacobian) override;
};


#endif //RAMPACK_PAIRBINNEDCORRELATION_H
