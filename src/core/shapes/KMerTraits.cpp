//
// Created by Piotr Kubala on 02/01/2021.
//

#include "KMerTraits.h"
#include "utils/Assertions.h"

std::vector<KMerTraits::SphereData> KMerTraits::generateSphereData(std::size_t sphereNum, double sphereRadius,
                                                                   double distance)
{
    Expects(sphereNum >= 2);
    Expects(sphereRadius > 0);
    Expects(distance > 0);

    std::vector<SphereData> data;
    data.reserve(sphereNum);
    double sphereX = -static_cast<double>(sphereNum - 1)/2 * distance;
    for (std::size_t i{}; i < sphereNum; i++) {
        data.emplace_back(SphereData({sphereX, 0, 0}, sphereRadius));
        sphereX += distance;
    }
    return data;
}
