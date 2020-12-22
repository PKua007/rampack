//
// Created by Piotr Kubala on 22/12/2020.
//

#include "PolysphereBananaTraits.h"
#include "utils/Assertions.h"

#include <iostream>

std::vector<PolysphereTraits::SphereData> PolysphereBananaTraits::generateSphereData(double arcRadius, double arcAngle,
                                                                                     std::size_t sphereNum,
                                                                                     double sphereRadius)
{
    Expects(arcRadius > 0);
    Expects(arcAngle > 0);
    Expects(sphereNum > 0);
    Expects(sphereRadius > 0);

    double angleFactor = arcAngle/sphereNum;
    int start = static_cast<int>(-sphereNum/2);
    int end = static_cast<int>((sphereNum % 2 == 0) ? (sphereNum/2 - 1) : sphereNum/2);
    std::vector<SphereData> sphereData;
    sphereData.reserve(sphereNum);
    for (int i = start; i <= end; i++) {
        Vector<3> pos = Matrix<3, 3>::rotation(0, 0, i*angleFactor) * Vector<3>{-arcRadius, 0, 0};
        pos += Vector<3>{arcRadius, 0, 0};
        sphereData.emplace_back(pos, sphereRadius);
    }

    for (auto data : sphereData) {
        std::cout << "Sphere[" << data.position << ", " << data.radius << "]," << std::endl;
    }

    return sphereData;
}
