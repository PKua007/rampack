//
// Created by pkua on 19.05.22.
//

#include "LatticeTraits.h"


std::array<std::size_t, 3> LatticeTraits::parseAxisOrder(const std::string &axisOrderString) {
    if (axisOrderString == "xyz")
        return {0, 1, 2};
    else if (axisOrderString == "xzy")
        return {0, 2, 1};
    else if (axisOrderString == "yxz")
        return {1, 0, 2};
    else if (axisOrderString == "yzx")
        return {1, 2, 0};
    else if (axisOrderString == "zxy")
        return {2, 0, 1};
    else if (axisOrderString == "zyx")
        return {2, 1, 0};
    else
        throw PreconditionException("Malformed axis order");
}

std::size_t LatticeTraits::axisToIndex(LatticeTraits::Axis axis) {
    return static_cast<std::size_t>(axis);
}
