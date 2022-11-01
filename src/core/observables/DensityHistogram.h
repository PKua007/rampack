//
// Created by pkua on 31.10.22.
//

#ifndef RAMPACK_DENSITYHISTOGRAM_H
#define RAMPACK_DENSITYHISTOGRAM_H

#include <array>
#include <optional>

#include "core/BulkObservable.h"
#include "GoldstoneTracker.h"
#include "trackers/DummyTracker.h"
#include "utils/HistogramBuilder.h"


class DensityHistogram : public BulkObservable {
private:
    std::array<std::size_t, 3> numBins{};
    std::unique_ptr<GoldstoneTracker> tracker;
    HistogramBuilder3D histogramBuilder;
    std::optional<Vector<3>> firstOrigin;
    std::size_t numThreads{};

    static std::array<std::size_t, 3> normalizeNumBins(std::array<std::size_t, 3> array);

public:
    explicit DensityHistogram(const std::array<std::size_t, 3> &numBins,
                              std::unique_ptr<GoldstoneTracker> tracker = std::make_unique<DummyTracker>(),
                              std::size_t numThreads = 1);

    void addSnapshot(const Packing &packing, double temperature, double pressure,
                     const ShapeTraits &shapeTraits) override;
    void print(std::ostream &out) const override;
    void clear() override;
    [[nodiscard]] std::string getSignatureName() const override { return "density_histogram"; };

    [[nodiscard]] std::vector<HistogramBuilder3D::BinValue> dumpValues() const;
};


#endif //RAMPACK_DENSITYHISTOGRAM_H
