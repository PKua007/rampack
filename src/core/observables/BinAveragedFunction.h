//
// Created by Piotr Kubala on 15/09/2023.
//

#ifndef RAMPACK_BINAVERAGEDFUNCTION_H
#define RAMPACK_BINAVERAGEDFUNCTION_H

#include <array>
#include <optional>
#include <valarray>

#include "core/BulkObservable.h"
#include "GoldstoneTracker.h"
#include "HistogramBuilder.h"
#include "ShapeFunction.h"


/**
 * @brief Divides the system into bins and averages some observable over those bins and over the system snapshots.
 * @details The class may use GoldstoneTracker to accommodate for the movement of the system. The initial captured
 * snapshot fixes system origin position and its movement in subsequent snapshots is cancelled.
 */
class BinAveragedFunction : public BulkObservable {
private:
    std::array<std::size_t, 3> numBins{};
    std::shared_ptr<GoldstoneTracker> tracker;
    HistogramBuilder<3, std::valarray<double>> histogramBuilder;
    std::optional<Vector<3>> firstOrigin;
    OMP_MAYBE_UNUSED std::size_t numThreads{};  // maybe_unused for builds without OpenMP support
    std::shared_ptr<ShapeFunction> shapeFunction;
    bool printCount;

    static std::array<std::size_t, 3> normalizeNumBins(std::array<std::size_t, 3> array);
    static std::valarray<double> makeInitialValarray(const ShapeFunction &shapeFunction);

public:
    /**
     * @brief Creates the class.
     * @param numBins number of bins in each direction. One can specify 0 or 1 in a given direction to turn off binning
     * in this direction.
     * @param shapeFunction shape function to be averaged in the bins. It can be multivalued
     * @param tracker GoldstoneTracker used to track the movement of the system. If @a nullptr is specified, no tracking
     * will be performed
     * @param printCount if @a true, bin count will be additionally printed in the output
     * @param numThreads number of threads used to generate the histogram. If 0, all available threads will be used
     */
    explicit BinAveragedFunction(const std::array<std::size_t, 3> &numBins,
                                 std::shared_ptr<ShapeFunction> shapeFunction,
                                 std::shared_ptr<GoldstoneTracker> tracker = nullptr, bool printCount = false,
                                 std::size_t numThreads = 1);

    void addSnapshot(const Packing &packing, double temperature, double pressure,
                     const ShapeTraits &shapeTraits) override;

    /**
     * @brief Prints averaged function values as space-separated rows to @a out output stream.
     * @details Each row corresponds to a single bin with format
     * @code
     * [x bin middle] [y ...] [z ...] [function value 1] [function value 2] ... [print count]
     * @endcode
     * The last `[print count]` column is outputted only if @a printCount was set @a true in the constructor.
     */
    void print(std::ostream &out) const override;

    void clear() override;

    /**
     * @brief Returns `[shape function primary name]_xyz` as the signature name.
     */
    [[nodiscard]] std::string getSignatureName() const override {
        return this->shapeFunction->getPrimaryName() + "_xyz";
    }

    /**
     * @brief Dumps a flat list of HistogramBuilder3D::BinValue -s averaged over snapshots.
     */
    [[nodiscard]] std::vector<Histogram<3, std::valarray<double>>::BinValue> dumpValues() const;
};


#endif //RAMPACK_BINAVERAGEDFUNCTION_H
