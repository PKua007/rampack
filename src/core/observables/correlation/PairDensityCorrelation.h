//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_PAIRDENSITYCORRELATION_H
#define RAMPACK_PAIRDENSITYCORRELATION_H

#include <memory>

#include "core/BulkObservable.h"
#include "PairConsumer.h"
#include "PairEnumerator.h"
#include "core/observables/HistogramBuilder.h"


/**
 * @brief A BulkObservable calculating pair density correlation vs distance (for a given distance, the ratio of the
 * actual number of molecules in the packing and expected number assuming the packing is homogenous).
 * @details If no correlations are present, it is equal 1. Values smaller than 1 mean negative correlations while
 * greater than 1 - positive correlations. Distances are computed as per PairEnumerator implementation.
 */
class PairDensityCorrelation : public BulkObservable, protected PairConsumer {
private:
    std::shared_ptr<PairEnumerator> pairEnumerator;
    HistogramBuilder<1> histogram;
    bool printCount{};

    void consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                     const Vector<3> &distanceVector, const ShapeTraits &shapeTraits) override;

public:
    /**
     * @brief Construct the class for the given @a pairEnumerator.
     * @details It will gather data for distances from 0 to @a maxR using @a numBins beans and doing it in parallel
     * using @a numThreads, if @a pairEnumerator supports concurrency.
     */
    explicit PairDensityCorrelation(std::shared_ptr<PairEnumerator> pairEnumerator, double maxR, std::size_t numBins,
                                    bool printCount = false, std::size_t numThreads = 1)
            : PairConsumer(numThreads), pairEnumerator{std::move(pairEnumerator)},
              histogram(0, maxR, numBins, numThreads), printCount{printCount}
    { }

    void addSnapshot(const Packing &packing, double temperature, double pressure,
                     const ShapeTraits &shapeTraits) override;

    /**
     * @brief Prints pair density correlation function vs distance onto @a out stream as space separated columns
     * (distance, value, count). The last column is optional (as per @a printCount from the constructor).
     * @details The output looks like this:
     * @code
     * distance1 value1 count1
     * distance2 value2 count2
     * ...
     * dinstanceN valueN countN
     * @endcode
     */
    void print(std::ostream &out) const override;

    void clear() override { this->histogram.clear(); }

    /**
     * @brief Returns a signature name in a format "rho_[pair enumerator name]".
     */
    [[nodiscard]] std::string getSignatureName() const override {
        return "rho_" + this->pairEnumerator->getSignatureName();
    }
};


#endif //RAMPACK_PAIRDENSITYCORRELATION_H
