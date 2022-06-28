//
// Created by pkua on 22.02.2022.
//

#include "TriclinicBoxScalerFactory.h"
#include "core/VolumeScaler.h"
#include "utils/Utils.h"
#include "utils/Assertions.h"
#include "core/volume_scalers/DeltaVolumeScaler.h"
#include "core/volume_scalers/AnisotropicVolumeScaler.h"
#include "core/volume_scalers/LinearScalingFactorSampler.h"
#include "core/volume_scalers/LogScalingFactorSampler.h"
#include "core/volume_scalers/TriclinicAdapter.h"
#include "core/volume_scalers/TriclinicDeltaScaler.h"

namespace {
    std::unique_ptr<VolumeScaler> create_volume_scaler(std::string scalingType) {
        std::string independentString = "independent ";
        bool independent = startsWith(scalingType, independentString);
        if (independent)
            scalingType = scalingType.substr(independentString.length());

        const auto &X = AnisotropicVolumeScaler::X;
        const auto &Y = AnisotropicVolumeScaler::Y;
        const auto &Z = AnisotropicVolumeScaler::Z;

        auto linear = std::make_unique<LinearScalingFactorSampler>();
        auto log = std::make_unique<LogScalingFactorSampler>();

        // Old delta V scaling
        if (scalingType == "delta V")
            return std::make_unique<DeltaVolumeScaler>();
        // Linear scaling
        else if (scalingType == "linear isotropic")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(linear), X && Y && Z, independent);
        else if (scalingType == "linear anisotropic x")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(linear), X || (Y && Z), independent);
        else if (scalingType == "linear anisotropic y")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(linear), Y || (Z && X), independent);
        else if (scalingType == "linear anisotropic z")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(linear), Z || (X && Y), independent);
        else if (scalingType == "linear anisotropic xyz")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(linear), X || Y || Z, independent);
        // Log scaling
        else if (scalingType == "log isotropic")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(log), X && Y && Z, independent);
        else if (scalingType == "log anisotropic x")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(log), X || (Y && Z), independent);
        else if (scalingType == "log anisotropic y")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(log), Y || (Z && X), independent);
        else if (scalingType == "log anisotropic z")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(log), Z || (X && Y), independent);
        else if (scalingType == "log anisotropic xyz")
            return std::make_unique<AnisotropicVolumeScaler>(std::move(log), X || Y || Z, independent);
        else
            throw ValidationException("Unknown scaling type: " + scalingType);
    }
}

std::unique_ptr<TriclinicBoxScaler>TriclinicBoxScalerFactory::create(const std::string &scalingType) {
    std::string scalingTypeStripped = scalingType;
    std::string independentString = "independent ";
    bool scaleTogether = !startsWith(scalingType, independentString);
    if (!scaleTogether)
        scalingTypeStripped = scalingType.substr(independentString.length());

    if (scalingTypeStripped == "delta triclinic")
        return std::make_unique<TriclinicDeltaScaler>(scaleTogether);
    else
        return std::make_unique<TriclinicAdapter>(create_volume_scaler(scalingType));
}
