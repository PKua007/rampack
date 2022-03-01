//
// Created by pkua on 01.03.2022.
//

#include "PolysphereLollipopTraits.h"

#include "utils/Assertions.h"

std::vector<PolysphereTraits::SphereData> PolysphereLollipopTraits::generateSphereData(std::size_t sphereNum,
                                                                                       double smallSphereRadius,
                                                                                       double largeSphereRadius,
                                                                                       double smallSpherePenetration,
                                                                                       double largeSpherePenetration)
{
    Expects(sphereNum >= 2);
    Expects(smallSphereRadius > 0);
    Expects(largeSphereRadius > 0);
    Expects(smallSpherePenetration < 2*smallSphereRadius);
    Expects(largeSpherePenetration < 2*std::min(smallSphereRadius, largeSphereRadius));

    std::vector<SphereData> data;

    double centrePos{};
    for (std::size_t i{}; i < sphereNum - 1; i++) {
        data.emplace_back(Vector<3>{centrePos, 0, 0}, smallSphereRadius);
        if (i < sphereNum - 2)
            centrePos += 2*smallSphereRadius - smallSpherePenetration;
    }

    centrePos += smallSphereRadius + largeSphereRadius - largeSpherePenetration;
    data.emplace_back(Vector<3>{centrePos, 0, 0}, largeSphereRadius);

    return data;
}
