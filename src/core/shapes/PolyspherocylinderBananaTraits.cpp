//
// Created by Piotr Kubala on 26/04/2021.
//

#include "PolyspherocylinderBananaTraits.h"

#include "utils/Exceptions.h"


void PolyspherocylinderBananaTraits::basicValidation(double arcRadius, double arcAngle, std::size_t segmentsNum,
                                                     double radius)
{
    Expects(arcRadius > 0);
    Expects(arcAngle > 0);
    Expects(arcAngle < 2*M_PI);
    Expects(segmentsNum >= 2);
    Expects(radius > 0);
}

bool PolyspherocylinderBananaTraits::isArcOriginOutside(double arcRadius, double arcAngle, std::size_t segmentsNum,
                                                        double radius)
{
    PolyspherocylinderBananaTraits::basicValidation(arcRadius, arcAngle, segmentsNum, radius);
    double segmentAngle = arcAngle/static_cast<double>(segmentsNum);
    double segmentDistance = arcRadius * std::cos(segmentAngle/2);
    constexpr double EPSILON = 1e-12;
    return segmentDistance * (1 + EPSILON) >= radius;
}

bool PolyspherocylinderBananaTraits::isArcOpen(double arcRadius, double arcAngle, std::size_t segmentsNum,
                                               double radius)
{
    PolyspherocylinderBananaTraits::basicValidation(arcRadius, arcAngle, segmentsNum, radius);

    if (arcAngle <= M_PI)
        return true;

    double endDistance = 2 * std::sin(arcAngle/2) * arcRadius;
    constexpr double EPSILON = 1e-12;
    return endDistance * (1 + EPSILON) >= 2*radius;
}

double PolyspherocylinderBananaTraits::calculateVolume(double arcRadius, double arcAngle, std::size_t segmentsNum,
                                                       double radius)
{
    double R = arcRadius;
    double r = radius;
    auto n = static_cast<double>(segmentsNum);
    double alpha = arcAngle/n;

    double H = r * std::tan(alpha/2);
    double L = 2*R * std::sin(alpha/2);

    double V1 = M_PI*L*r*r;
    double V2 = 2./3*H*r*r;
    double V3 = 4./3*M_PI*r*r*r;

    return n*V1 - (2*n - 2)*V2 + ((n-1)*alpha/2/M_PI + 1)*V3;
}

PolyspherocylinderBananaTraits::PolyspherocylinderGeometry
PolyspherocylinderBananaTraits::generateGeometry(double arcRadius, double arcAngle, std::size_t segmentsNum,
                                                 double radius, std::size_t subdivisions)
{
    PolyspherocylinderBananaTraits::basicValidation(arcRadius, arcAngle, segmentsNum, radius);
    if (segmentsNum > 2)
        Expects(PolyspherocylinderBananaTraits::isArcOriginOutside(arcRadius, arcAngle, segmentsNum, radius));
    Expects(PolyspherocylinderBananaTraits::isArcOpen(arcRadius, arcAngle, segmentsNum, radius));
    Expects(subdivisions > 0);

    double angleStep = arcAngle/static_cast<double>(segmentsNum);
    double startAngle = -angleStep * static_cast<double>(segmentsNum - 1) / 2;
    std::vector<SpherocylinderData> scData;
    scData.reserve(segmentsNum);
    double angle = startAngle;
    for (std::size_t i{}; i < segmentsNum; i++) {
        Vector<3> pos1 = Matrix<3, 3>::rotation(0, angle - angleStep/2, 0) * Vector<3>{-arcRadius, 0, 0};
        Vector<3> pos2 = Matrix<3, 3>::rotation(0, angle + angleStep/2, 0) * Vector<3>{-arcRadius, 0, 0};

        Vector<3> axis = pos2 - pos1;
        Vector<3> halfAxis = axis / (2 * static_cast<double>(subdivisions));
        for (std::size_t j{}; j < subdivisions; j++) {
            Vector<3> pos = pos1 + axis * ((static_cast<double>(j) + 0.5) / static_cast<double>(subdivisions));
            scData.emplace_back(pos, halfAxis, radius);
        }
        angle += angleStep;
    }

    if (arcAngle < M_PI) {
        const auto &scFront = scData.front();
        Vector<3> beg = scFront.position - scFront.halfAxis;
        Vector<3> translation = {-beg[0], 0, 0};
        std::vector<SpherocylinderData> newScData;
        newScData.reserve(scData.size());
        for (auto &data : scData)
            newScData.emplace_back(data.position + translation, data.halfAxis, data.radius);
        std::swap(scData, newScData);
    }

    double volume = PolyspherocylinderBananaTraits::calculateVolume(arcRadius, arcAngle, segmentsNum, radius);
    PolyspherocylinderGeometry geometry(scData, {0, 0, 1}, {-1, 0, 0}, {0, 0, 0}, volume);
    geometry.addCustomNamedPoints({{"beg", scData.front().position - scData.front().halfAxis},
                                   {"end", scData.back().position + scData.back().halfAxis}});
    return geometry;
}
