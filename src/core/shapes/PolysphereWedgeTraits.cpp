//
// Created by pkua on 09.03.2022.
//

#include "PolysphereWedgeTraits.h"
#include "utils/Assertions.h"

std::vector<PolysphereTraits::SphereData> PolysphereWedgeTraits::generateSphereData(std::size_t sphereNum,
                                                                                    double smallSphereRadius,
                                                                                    double largeSphereRadius,
                                                                                    double spherePenetration)
{
    Expects(sphereNum >= 2);
    Expects(smallSphereRadius > 0);
    Expects(largeSphereRadius > 0);
    Expects(spherePenetration < 2*std::min(smallSphereRadius, largeSphereRadius));

    std::vector<SphereData> data;

    double centrePos{};
    double radiusDelta = (largeSphereRadius - smallSphereRadius)/static_cast<double>(sphereNum - 1);
    double currentRadius = smallSphereRadius;
    for (std::size_t i{}; i < sphereNum; i++) {
        data.emplace_back(Vector<3>{centrePos, 0, 0}, currentRadius);
        centrePos += 2*currentRadius + radiusDelta - spherePenetration;
        currentRadius += radiusDelta;
    }

    return data;
}
