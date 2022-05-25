//
// Created by Piotr Kubala on 22/12/2020.
//

#include "PolysphereBananaTraits.h"
#include "utils/Assertions.h"

PolysphereBananaTraits::PolysphereGeometry
PolysphereBananaTraits::generateGeometry(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius,
                                         bool normalizeMassCentre)
{
    Expects(arcRadius > 0);
    Expects(arcAngle > 0);
    Expects(sphereNum >= 2);
    Expects(sphereRadius > 0);

    double angleFactor = arcAngle/static_cast<double>(sphereNum - 1);
    int start = -static_cast<int>(sphereNum)/2;
    int end = static_cast<int>((sphereNum % 2 == 0) ? (sphereNum/2 - 1) : sphereNum/2);
    std::vector<SphereData> sphereData;
    sphereData.reserve(sphereNum);
    for (int i = start; i <= end; i++) {
        Vector<3> pos = Matrix<3, 3>::rotation(0, 0, -i*angleFactor) * Vector<3>{-arcRadius, 0, 0};
        pos += Vector<3>{arcRadius, 0, 0};
        sphereData.emplace_back(pos, sphereRadius);
    }

    PolysphereGeometry geometry(std::move(sphereData), {0, 1, 0}, {-1, 0, 0});
    if (normalizeMassCentre) {
        geometry.normalizeMassCentre();
        geometry.setGeometricOrigin({0, 0, 0});
    } else {
        geometry.setGeometricOrigin(geometry.calculateMassCentre());
    }
    return geometry;
}
