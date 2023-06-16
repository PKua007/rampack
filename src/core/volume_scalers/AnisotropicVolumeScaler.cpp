//
// Created by pkua on 27.06.22.
//

#include <algorithm>
#include <functional>

#include "AnisotropicVolumeScaler.h"
#include "utils/Exceptions.h"
#include "utils/CompilerMacros.h"


const AnisotropicVolumeScaler::ScalingDirection AnisotropicVolumeScaler::X({{true, false, false}});
const AnisotropicVolumeScaler::ScalingDirection AnisotropicVolumeScaler::Y({{false, true, false}});
const AnisotropicVolumeScaler::ScalingDirection AnisotropicVolumeScaler::Z({{false, false, true}});


std::array<double, 3> AnisotropicVolumeScaler::sampleScalingFactors(const std::array<double, 3> &oldDim,
                                                                    double scalingStepSize, std::mt19937 &mt) const
{
    if (this->independent && this->scalingDirection.size() > 1) {
        std::uniform_int_distribution<std::size_t> directionSampler(0, this->scalingDirection.size() - 1);
        auto shouldSample = this->scalingDirection.directions[directionSampler(mt)];
        return this->factorSampler->sampleFactors(shouldSample, oldDim, scalingStepSize, mt);
    } else {
        std::array<double, 3> factors = {1, 1, 1};
        for (auto shouldSample : this->scalingDirection.directions) {
            auto singleFactors = this->factorSampler->sampleFactors(shouldSample, oldDim, scalingStepSize, mt);
            std::transform(factors.begin(), factors.end(), singleFactors.begin(), factors.begin(), std::multiplies<>{});
        }
        return factors;
    }
}

AnisotropicVolumeScaler::ScalingDirection operator|(const AnisotropicVolumeScaler::ScalingDirection &sd1,
                                                    const AnisotropicVolumeScaler::ScalingDirection &sd2)
{
    AnisotropicVolumeScaler::ScalingDirection result = sd1;
    for (const auto &elem2 : sd2.directions)
        if (std::find(result.directions.begin(), result.directions.end(), elem2) == result.directions.end())
            result.directions.push_back(elem2);
    return result;
}

AnisotropicVolumeScaler::ScalingDirection operator&(const AnisotropicVolumeScaler::ScalingDirection &sd1,
                                                    const AnisotropicVolumeScaler::ScalingDirection &sd2)
{
// GCC 13 incorrectly reports a warning on malformed bounds in __builtin_memmove, similar to
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100366
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107852
#if defined(RAMPACK_GCC) && RAMPACK_GCC >= 130000
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Warray-bounds"
    #pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

    if (sd1.empty() && sd2.size() == 1)
        return sd2;
    if (sd2.empty() && sd1.size() == 1)
        return sd1;

#if defined(RAMPACK_GCC) && RAMPACK_GCC >= 130000
    #pragma GCC diagnostic pop
#endif

    Expects(sd1.size() == 1);
    Expects(sd2.size() == 1);

    const auto &dir1 = sd1.directions.front();
    const auto &dir2 = sd2.directions.front();
    std::array<bool, 3> result{};
    std::transform(dir1.begin(), dir1.end(), dir2.begin(), result.begin(), std::logical_or<>{});
    return {{result}};
}

AnisotropicVolumeScaler::AnisotropicVolumeScaler(std::unique_ptr<ScalingFactorSampler> factorSampler,
                                                 const AnisotropicVolumeScaler::ScalingDirection &scalingDirection,
                                                 bool independent)
        : factorSampler{std::move(factorSampler)}, scalingDirection{scalingDirection}, independent{independent}
{
    Expects(!scalingDirection.empty());
}
