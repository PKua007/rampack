//
// Created by Piotr Kubala on 06/01/2023.
//

#ifndef RAMPACK_SCALINGDIRECTIONPARSER_H
#define RAMPACK_SCALINGDIRECTIONPARSER_H

#include <stdexcept>

#include "core/volume_scalers/AnisotropicVolumeScaler.h"


class ScalingDirectionException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};


class ScalingDirectionParser {
private:
    static AnisotropicVolumeScaler::ScalingDirection charToScalingDirection(char c);
    static void registerScalingDirection(char direction, std::array<bool, 3> &isDirectionUsed);
    static std::string parseBrackets(char opening, char closing, const std::string &str, std::size_t &idx);

public:
    static AnisotropicVolumeScaler::ScalingDirection parse(const std::string &scalingDirectionStr);
};


#endif //RAMPACK_SCALINGDIRECTIONPARSER_H
