//
// Created by Piotr Kubala on 26/04/2021.
//

#include "PolyspherocylinderBananaTraits.h"

#include "utils/Assertions.h"

std::vector<PolyspherocylinderTraits::SpherocylinderData>
PolyspherocylinderBananaTraits::generateSpherocylinderData(double arcRadius, double arcAngle, std::size_t segmentsNum,
                                                           double radius, std::size_t subdivisions)
{
    Expects(arcRadius > 0);
    Expects(arcAngle > 0);
    Expects(segmentsNum > 0);
    Expects(subdivisions > 0);
    Expects(radius > 0);

    double angleStep = arcAngle/static_cast<double>(segmentsNum);
    double startAngle = -angleStep * static_cast<double>(segmentsNum - 1) / 2;
    std::vector<SpherocylinderData> spherocylinderData;
    spherocylinderData.reserve(segmentsNum);
    double angle = startAngle;
    for (std::size_t i{}; i < segmentsNum; i++) {
        Vector<3> pos1 = Matrix<3, 3>::rotation(0, 0, angle - angleStep/2) * Vector<3>{-arcRadius, 0, 0};
        Vector<3> pos2 = Matrix<3, 3>::rotation(0, 0, angle + angleStep/2) * Vector<3>{-arcRadius, 0, 0};

        Vector<3> axis = pos2 - pos1;
        Vector<3> halfAxis = axis / (2 * static_cast<double>(subdivisions));
        for (std::size_t j{}; j < subdivisions; j++) {
            Vector<3> pos = pos1 + axis * ((static_cast<double>(j) + 0.5) / static_cast<double>(subdivisions));
            spherocylinderData.emplace_back(pos, halfAxis, radius);
        }
        angle += angleStep;
    }

    return spherocylinderData;
}
