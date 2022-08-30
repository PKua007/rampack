//
// Created by pkua on 01.03.2022.
//

#include "PolysphereLollipopTraits.h"

#include "utils/Assertions.h"

PolysphereLollipopTraits::PolysphereGeometry
PolysphereLollipopTraits::generateGeometry(std::size_t sphereNum, double smallSphereRadius,
                                           double largeSphereRadius, double smallSpherePenetration,
                                           double largeSpherePenetration, bool normalizeMassCentre)
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

    PolysphereGeometry geometry(std::move(data), {1, 0, 0}, {0, 1, 0});
    if (normalizeMassCentre)
        geometry.normalizeMassCentre();
    double end1 = geometry.getSphereData().front().position[0];
    double end2 = geometry.getSphereData().back().position[0];
    geometry.setGeometricOrigin({(end2 + largeSphereRadius + end1 - smallSphereRadius)/2, 0, 0});
    geometry.setNamedPoints({{"ss", geometry.getSphereData().front().position},
                             {"sl", geometry.getSphereData().back().position}});
    return geometry;
}
