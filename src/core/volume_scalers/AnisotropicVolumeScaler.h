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


/**
 * @brief A general VolumeScaler with customizable scaling directions and scaling steps sampled by ScalingFactorSampler.
 * @details Scaling directions are represented by helper ScalingDirections class. The direction is selected from
 * alternatives (see ScalingDirection description). Each alternative can contain scaling in one or more directions
 * simultaneously. Alternatives can be sampled one at a time, or all combined (switch @a independent in the
 * constructor).
 */
class AnisotropicVolumeScaler : public VolumeScaler {
public:
    /**
     * @brief A class representing alternatives for scaling directions.
     * @details Scaling direction are created by combining directions with ScalingDirection::operator|=() and
     * ScalingDirection::operator&=() operators. The basic building blocks are constants
     * AnisotropicVolumeScaler::X, AnisotropicVolumeScaler::Y and AnisotropicVolumeScaler::Z. Combining them with
     * @a & operator represents directions scaled at the same time, not independently (using the same random number). On
     * the other hand, directions combined with @a | operator represents alternative scaling directions, which are
     * selected independently, one at a time. Directions combined with @a & can be further combined with @a |, but not
     * the other way round. As an example
     * @code
     * (X & Y) | Z
     * @endcode
     * Gives scaling alternatively: x and y together or scaling only z. Moreover, not every direction has to be
     * specified - then, it is never scaled.
     */
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

    /**
     * @brief Samples scaling factor using @a factorSampler, basing on alternatives from @a scalingDirection.
     * @details Final scaling factors are computed as such: if @a independent is @a true, one of @a | alternatives
     * from @a scalingDirection is selected and random. All directions from this alternative are sampled using the same
     * random number (in a @a factorSampler dependent way). However, if @a independent is @a false, all alternatives
     * are sampled (each with independent random number) are factors are combined. Examples:
     * <ol>
     * <li> <em>scalingDirection = X&Y|Z</em>, <em>independent = true</em>: <br>
     *     either X and Y are sampled simultaneously (the same scaling step) or Z
     * <li> <em>scalingDirection = X|Y</em>, <em>independent = false</em>: <br>
     *     X, Y are sampled simultaneously, but each using a different random number (Z is never scaled)
     * <li> <em>scalingDirection = X&Y|Z</em>, <em>independent = false</em>: <br>
     *     all X, Y, Z are scaled, but X and Y using the same random number and Z using a different, independent one
     * </ol>
     */
    AnisotropicVolumeScaler(std::unique_ptr<ScalingFactorSampler> factorSampler,
                            const ScalingDirection &scalingDirection, bool independent = false);

    /**
     * @brief Samples scaling factors, capped by @a scalingStepSize for a box of old dimensions @a oldDim using @a mt
     * RNG.
     */
    [[nodiscard]] std::array<double, 3> sampleScalingFactors(const std::array<double, 3> &oldDim,
                                                             double scalingStepSize, std::mt19937 &mt) const override;
};


#endif //RAMPACK_ANISOTROPICVOLUMESCALER_H
