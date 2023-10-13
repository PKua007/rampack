//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_PAIRAVERAGEDCORRELATION_H
#define RAMPACK_PAIRAVERAGEDCORRELATION_H

#include <memory>

#include "core/BulkObservable.h"
#include "PairConsumer.h"
#include "PairEnumerator.h"
#include "core/observables/HistogramBuilder.h"
#include "core/observables/CorrelationFunction.h"


/**
 * @brief Bulk observable which computes an average of the given CorrelationFunction versus the distance between
 * molecules as per given PairEnumerator.
 */
class PairAveragedCorrelation : public BulkObservable, public PairConsumer {
private:
    std::shared_ptr<PairEnumerator> pairEnumerator;
    std::shared_ptr<CorrelationFunction> correlationFunction;
    HistogramBuilder<1> histogram;
    bool printCount{};

    void consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                     const Vector<3> &distanceVector, const ShapeTraits &shapeTraits) override;

public:
    /**
     * @brief Construct the class for the given @a pairEnumerator and @a correlationFunction.
     * @details It will gather data for distances from 0 to @a maxR using @a numBins beans and doing it in parallel
     * using @a numThreads, if @a pairEnumerator supports concurrency.
     * @param pairEnumerator PairEnumerator used
     * @param correlationFunction CorrelationFunction to be averaged
     * @param maxR maximal distance (as per @a pairEnumerator) to gather values for
     * @param numBins number of bins in the range [0, @a maxR]
     * @param printCount if @a true, raw bin count will be printed in addition to function values
     * @param numThreads number of concurrent threads that will be supported
     */
    PairAveragedCorrelation(std::shared_ptr<PairEnumerator> pairEnumerator,
                            std::shared_ptr<CorrelationFunction> correlationFunction, double maxR, std::size_t numBins,
                            bool printCount = false, std::size_t numThreads = 1)
            : PairConsumer(numThreads), pairEnumerator{std::move(pairEnumerator)},
              correlationFunction{std::move(correlationFunction)}, histogram(0, maxR, numBins, numThreads),
              printCount{printCount}
    { }

    void addSnapshot(const Packing &packing, double temperature, double pressure,
                     const ShapeTraits &shapeTraits) override;

    /**
     * @brief Prints correlation function vs distance onto @a out stream as space separated columns (distance, value,
     * count). The last column is optional (as per @a printCount from the constructor).
     * @details The output looks like this:
     * @code
     * distance1 value1
     * distance2 value2
     * ...
     * distanceN valueN
     * @endcode
     */
    void print(std::ostream &out) const override;

    void clear() override { this->histogram.clear(); }

    /**
     * @brief Returns a signature name in a format "[correlation function name]_[pair enumerator name]".
     */
    [[nodiscard]] std::string getSignatureName() const override {
        return this->correlationFunction->getSignatureName() + "_" + this->pairEnumerator->getSignatureName();
    }
};


#endif //RAMPACK_PAIRAVERAGEDCORRELATION_H
