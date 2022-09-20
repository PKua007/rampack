//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_PAIRDENSITYCORRELATION_H
#define RAMPACK_PAIRDENSITYCORRELATION_H

#include <memory>

#include "core/BulkObservable.h"
#include "PairConsumer.h"
#include "PairEnumerator.h"
#include "HistogramBuilder.h"


class PairDensityCorrelation : public BulkObservable, protected PairConsumer {
private:
    std::unique_ptr<PairEnumerator> pairEnumerator;
    HistogramBuilder histogram;

protected:
    void consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                     double distance, const ShapeTraits &shapeTraits) override;

public:
    explicit PairDensityCorrelation(std::unique_ptr<PairEnumerator> pairEnumerator, double maxR, std::size_t numBins,
                                    std::size_t numThreads = 1)
            : PairConsumer(numThreads), pairEnumerator{std::move(pairEnumerator)},
              histogram(0, maxR, numBins, numThreads)
    { }

    void addSnapshot(const Packing &packing, double temperature, double pressure,
                     const ShapeTraits &shapeTraits) override;
    void print(std::ostream &out) const override;
    void clear() override { this->histogram.clear(); }

    [[nodiscard]] std::string getSignatureName() const override {
        return "rho_" + this->pairEnumerator->getSignatureName();
    }
};


#endif //RAMPACK_PAIRDENSITYCORRELATION_H
