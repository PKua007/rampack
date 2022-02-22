//
// Created by pkua on 22.02.2022.
//

#include "TriclinicBoxScalerFactory.h"
#include "core/VolumeScaler.h"
#include "utils/Utils.h"
#include "utils/Assertions.h"
#include "core/volume_scalers/DeltaVolumeScaler.h"
#include "core/volume_scalers/LinearVolumeScaler.h"
#include "core/volume_scalers/LogVolumeScaler.h"
#include "core/volume_scalers/TriclinicAdapter.h"
#include "core/volume_scalers/TriclinicDeltaScaler.h"

namespace {
    std::unique_ptr<VolumeScaler> create_volume_scaler(std::string scalingType) {
        std::string independentString = "independent ";
        bool scaleTogether = !startsWith(scalingType, independentString);
        if (!scaleTogether)
            scalingType = scalingType.substr(independentString.length());

        using ScalingDirection = VolumeScaler::ScalingDirection;

        // Old delta V scaling
        if (scalingType == "delta V")
            return std::make_unique<DeltaVolumeScaler>();
        // Linear scaling
        else if (scalingType == "linear isotropic")
            return std::make_unique<LinearVolumeScaler>(ScalingDirection::ISOTROPIC);
        else if (scalingType == "linear anisotropic x")
            return std::make_unique<LinearVolumeScaler>(ScalingDirection::ANISOTROPIC_X);
        else if (scalingType == "linear anisotropic y")
            return std::make_unique<LinearVolumeScaler>(ScalingDirection::ANISOTROPIC_Y);
        else if (scalingType == "linear anisotropic z")
            return std::make_unique<LinearVolumeScaler>(ScalingDirection::ANISOTROPIC_Z);
        else if (scalingType == "linear anisotropic xyz")
            return std::make_unique<LinearVolumeScaler>(ScalingDirection::ANISOTROPIC_XYZ);
        // Log scaling
        else if (scalingType == "log isotropic")
            return std::make_unique<LogVolumeScaler>(ScalingDirection::ISOTROPIC, scaleTogether);
        else if (scalingType == "log anisotropic x")
            return std::make_unique<LogVolumeScaler>(ScalingDirection::ANISOTROPIC_X, scaleTogether);
        else if (scalingType == "log anisotropic y")
            return std::make_unique<LogVolumeScaler>(ScalingDirection::ANISOTROPIC_Y, scaleTogether);
        else if (scalingType == "log anisotropic z")
            return std::make_unique<LogVolumeScaler>(ScalingDirection::ANISOTROPIC_Z, scaleTogether);
        else if (scalingType == "log anisotropic xyz")
            return std::make_unique<LogVolumeScaler>(ScalingDirection::ANISOTROPIC_XYZ, scaleTogether);
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
