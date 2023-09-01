//
// Created by Piotr Kubala on 20/04/2023.
//

#ifndef RAMPACK_PROBABILITYEVOLUTION_H
#define RAMPACK_PROBABILITYEVOLUTION_H

#include "core/observables/HistogramBuilder.h"
#include "core/BulkObservable.h"
#include "PairConsumer.h"
#include "PairEnumerator.h"
#include "core/observables/CorrelationFunction.h"


/**
 * @brief Two-dimensional histogram showing the dependence of probability distribution of a given correlation function
 * on the distance between particles.
 * @details It can be seen as a generalization of PairAveragedCorrelation which shows the whole probability distribution
 * function instead only the average value.
 */
class ProbabilityEvolution : public BulkObservable, public PairConsumer {
public:
    /**
     * @brief How probability distribution for a given distance should be normalized.
     */
    enum class Normalization {
        /** @brief No normalization is performed - each bin contains snapshot-averaged count of points. Note, that
         * the sum of counts for a given distance will vary with the distance (it will be proportional to
         * PairDensityCorrelation).
         */
        NONE,

        /** @brief Normalize the probability as the probability distribution function
         * \f$ \int_{f_\mathrm{min}}^{f_\mathrm{max}} P(f|r)\ \mathrm{d}f = 1\f$
         * for each distance \f$d\f$
         */
        PDF,

        /**
         * @brief Normalize the counts so that the average bin value for a given distance is 1.
         */
        UNIT
    };

private:
    double maxDistance{};
    std::pair<double, double> functionRange{};
    HistogramBuilder<2> histogramBuilder;
    std::shared_ptr<PairEnumerator> pairEnumerator;
    std::shared_ptr<CorrelationFunction> function;
    Normalization normalization{};

    void renormalizeHistogram(Histogram2D &histogram) const;
    void consumePair(const Packing &packing, const std::pair<std::size_t, std::size_t> &idxPair,
                     const Vector<3> &distanceVector, const ShapeTraits &shapeTraits) override;

public:
    /**
     * @brief Constructs the class.
     * @param maxDistance maximal distance between the particles to be plotted
     * @param numDistanceBins number of bins for the distances
     * @param pairEnumerator thePairEnumerator used for binning
     * @param functionRange the range the the CorrelationFunction to be plotted
     * @param numFunctionBins number of bins for the CorrelationFunction values
     * @param function the CorrelationFunctions to compute
     * @param normalization type of normalization
     * @param numThreads number of threads used to generate the histogram. If 0, all available threads will be used
     */
    ProbabilityEvolution(double maxDistance, std::size_t numDistanceBins,
                         std::shared_ptr<PairEnumerator> pairEnumerator, std::pair<double, double> functionRange,
                         std::size_t numFunctionBins, std::shared_ptr<CorrelationFunction> function,
                         Normalization normalization = Normalization::PDF, std::size_t numThreads = 1);

    void addSnapshot(const Packing &packing, double temperature, double pressure,
                     const ShapeTraits &shapeTraits) override;

    /**
     * @brief Output the histogram.
     * @details The format is
     * @code
     * [distance 1] [function value 1] [probability/count 1,1]
     * ...
     * [distance 1] [function value N] [probability/count 1,N]
     *
     *   ...
     *
     * [distance M] [function value 1] [probability/count M,1]
     * ...
     * [distance M] [function value N] [probability/count M,N]
     * @endcode
     */
    void print(std::ostream &out) const override;

    void clear() override;

    /**
     * @brief Returns the signature name, which is `prob_[correlation function name]_[pair enumerator name]`
     */
    [[nodiscard]] std::string getSignatureName() const override;
};


#endif //RAMPACK_PROBABILITYEVOLUTION_H
