//
// Created by pkua on 27.06.22.
//

#ifndef RAMPACK_ANISOTROPICVOLUMESCALER_H
#define RAMPACK_ANISOTROPICVOLUMESCALER_H

#include <memory>
#include <utility>
#include <vector>

#include "core/VolumeScaler.h"
#include "ScalingFactorSampler.h"


class AnisotropicVolumeScaler : public VolumeScaler {
public:
    class ScalingDirection {
    private:
        friend class AnisotropicVolumeScaler;

        std::vector<std::array<bool, 3>> directions;

        ScalingDirection(std::vector<std::array<bool, 3>> directions) : directions{std::move(directions)} { }

        [[nodiscard]] std::size_t size() const { return this->directions.size(); }
        [[nodiscard]] bool empty() const { return this->directions.empty(); }

    public:
        ScalingDirection() = default;

        friend ScalingDirection operator|(const ScalingDirection &sd1, const ScalingDirection &sd2);
        friend ScalingDirection operator&(const ScalingDirection &sd1, const ScalingDirection &sd2);

        ScalingDirection &operator|=(const ScalingDirection &sd2) { return *this = *this | sd2; };
        ScalingDirection &operator&=(const ScalingDirection &sd2) { return *this = *this & sd2; };
    };

private:
    std::unique_ptr<ScalingFactorSampler> factorSampler;
    ScalingDirection scalingDirection;
    bool independent{};

public:
    const static ScalingDirection X;
    const static ScalingDirection Y;
    const static ScalingDirection Z;

    AnisotropicVolumeScaler(std::unique_ptr<ScalingFactorSampler> factorSampler,
                            const ScalingDirection &scalingDirection, bool independent = false);

    [[nodiscard]] std::array<double, 3> sampleScalingFactors(const std::array<double, 3> &oldDim,
                                                             double scalingStepSize, std::mt19937 &mt) const override;
};


#endif //RAMPACK_ANISOTROPICVOLUMESCALER_H
