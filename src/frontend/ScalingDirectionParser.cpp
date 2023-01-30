//
// Created by Piotr Kubala on 06/01/2023.
//

#include <algorithm>

#include "ScalingDirectionParser.h"
#include "utils/Exceptions.h"


AnisotropicVolumeScaler::ScalingDirection ScalingDirectionParser::charToScalingDirection(char c) {
    switch (c) {
        case 'x':
            return AnisotropicVolumeScaler::X;
        case 'y':
            return AnisotropicVolumeScaler::Y;
        case 'z':
            return AnisotropicVolumeScaler::Z;
        default:
            AssertThrow(std::string{c});
    }
}

void ScalingDirectionParser::registerScalingDirection(char direction, std::array<bool, 3> &isDirectionUsed) {
    if (direction < 'x' || direction > 'z')
        throw ScalingDirectionException("Unexpected character: '" + std::string{direction} + "'");
    std::size_t directionIdx = direction - 'x';
    if (isDirectionUsed[directionIdx])
        throw ScalingDirectionException("Duplicated occurrence of " + std::string{direction} + " direction");
    isDirectionUsed[directionIdx] = true;
}

std::string ScalingDirectionParser::parseBrackets(char opening, char closing, const std::string &str, size_t &idx) {
    std::size_t closingIdx = str.find(closing, idx);
    if (closingIdx == std::string::npos)
        throw ScalingDirectionException("Unmatched '" + std::string{opening} + "' in scaling direction");
    std::string content = str.substr(idx + 1, closingIdx - idx - 1);
    idx = closingIdx;
    return content;
}

AnisotropicVolumeScaler::ScalingDirection
ScalingDirectionParser::parse(const std::string &scalingDirectionStr)
{
    AnisotropicVolumeScaler::ScalingDirection scalingDirection;
    std::array<bool, 3> isDirectionUsed{};
    for (std::size_t i{}; i < scalingDirectionStr.length(); i++) {
        char c = scalingDirectionStr[i];
        switch (c) {
            case 'x':
            case 'y':
            case 'z':
                registerScalingDirection(c, isDirectionUsed);
                scalingDirection |= charToScalingDirection(c);
                break;

            case '(': {
                AnisotropicVolumeScaler::ScalingDirection groupedScalingDirection;
                for (char direction : parseBrackets('(', ')', scalingDirectionStr, i)) {
                    registerScalingDirection(direction, isDirectionUsed);
                    groupedScalingDirection &= charToScalingDirection(direction);
                }
                scalingDirection |= groupedScalingDirection;
                break;
            }

            case '[':
                for (char direction : parseBrackets('[', ']', scalingDirectionStr, i))
                    registerScalingDirection(direction, isDirectionUsed);
                break;

            default:
                throw ScalingDirectionException("Unexpected character: '" + std::string{c} + "'");
        }
    }

    if (std::find(isDirectionUsed.begin(), isDirectionUsed.end(), false) != isDirectionUsed.end())
        throw ScalingDirectionException("The behaviour of one or more scaling directions is unspecified");

    return scalingDirection;
}
