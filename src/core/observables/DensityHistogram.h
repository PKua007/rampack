//
// Created by pkua on 31.10.22.
//

#ifndef RAMPACK_DENSITYHISTOGRAM_H
#define RAMPACK_DENSITYHISTOGRAM_H

#include <array>
#include <optional>

#include "core/BulkObservable.h"
#include "GoldstoneTracker.h"
#include "HistogramBuilder.h"


/**
 * @brief BulkObservable creating the density histogram.
 * @details The histogram is normalized by multiplying by the number of bins and dividing by the number of molecules.
 * The class may use GoldstoneTracker to accommodate for the movement of the system. The initial captured snapshot
 * fixes system origin position and its movement in subsequent snapshots is cancelled.
 */
class DensityHistogram : public BulkObservable {
private:
    std::array<std::size_t, 3> numBins{};
    std::shared_ptr<GoldstoneTracker> tracker;
    HistogramBuilder3D histogramBuilder;
    std::optional<Vector<3>> firstOrigin;
    [[maybe_unused]] std::size_t numThreads{};  // maybe_unused for builds without OpenMP support

    static std::array<std::size_t, 3> normalizeNumBins(std::array<std::size_t, 3> array);

public:
    /**
     * @brief Creates the class.
     * @param numBins number of bins in each direction. One can specify 0 or 1 in a given direction to turn off binning
     * in this direction.
     * @param tracker GoldstoneTracker used to track the movement of the system. If @a nullptr is specified, no tracking
     * will be performed
     * @param numThreads number of threads used to generate the histogram. If 0, all available threads will be used
     */
    explicit DensityHistogram(const std::array<std::size_t, 3> &numBins,
                              std::shared_ptr<GoldstoneTracker> tracker = nullptr, std::size_t numThreads = 1);

    void addSnapshot(const Packing &packing, double temperature, double pressure,
                     const ShapeTraits &shapeTraits) override;
    void print(std::ostream &out) const override;
    void clear() override;

    /**
     * @brief Returns "rho_xyz" as the signature name.
     */
    [[nodiscard]] std::string getSignatureName() const override { return "rho_xyz"; };

    /**
     * @brief Dumps a flat list of HistogramBuilder3D::BinValue -s averages over snapshots.
     */
    [[nodiscard]] std::vector<HistogramBuilder3D::BinValue> dumpValues() const;
};


#endif //RAMPACK_DENSITYHISTOGRAM_H
