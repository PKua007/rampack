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
 * @details Factors from VolumeScaler are translated into elongations/shrinking of corresponding triclinic box axes,
 * preserving the angles between box faces.
 */
class TriclinicAdapter : public TriclinicBoxScaler {
private:
    std::unique_ptr<VolumeScaler> volumeScaler;

public:
    TriclinicAdapter(std::unique_ptr<VolumeScaler> volumeScaler, double stepSize)
            : TriclinicBoxScaler(stepSize), volumeScaler{std::move(volumeScaler)}
    { }

    TriclinicBox updateBox(const TriclinicBox &oldBox, std::mt19937 &mt) const override {
        auto heights = oldBox.getHeights();
        auto factors = this->volumeScaler->sampleScalingFactors(heights, this->stepSize, mt);
        auto sides = oldBox.getSides();
        std::transform(sides.begin(), sides.end(), factors.begin(), sides.begin(), std::multiplies<>{});
        return TriclinicBox(sides);
    }
};


#endif //RAMPACK_TRICLINICADAPTER_H
