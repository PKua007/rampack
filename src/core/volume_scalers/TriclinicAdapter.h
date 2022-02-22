//
// Created by pkua on 22.02.2022.
//

#ifndef RAMPACK_TRICLINICADAPTER_H
#define RAMPACK_TRICLINICADAPTER_H

#include <memory>

#include "core/VolumeScaler.h"
#include "core/TriclinicBoxScaler.h"

/**
 * @brief Class transforming old-fashioned VolumeScaler into new TriclinicBoxScaler.
 */
class TriclinicAdapter : public TriclinicBoxScaler {
private:
    std::unique_ptr<VolumeScaler> volumeScaler;

public:
    explicit TriclinicAdapter(std::unique_ptr<VolumeScaler> volumeScaler) : volumeScaler{std::move(volumeScaler)} { }

    TriclinicBox updateBox(const TriclinicBox &oldBox, double scalingStepSize, std::mt19937 &mt) const override {
        TriclinicBox newBox = oldBox;
        auto heights = oldBox.getHeights();
        newBox.scale(this->volumeScaler->sampleScalingFactors(heights, scalingStepSize, mt));
        return newBox;
    }
};


#endif //RAMPACK_TRICLINICADAPTER_H
