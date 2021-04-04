//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_POLYSPHEREBANANATRAITS_H
#define RAMPACK_POLYSPHEREBANANATRAITS_H

#include "PolysphereTraits.h"

class PolysphereBananaTraits : public PolysphereTraits {
private:
    static std::vector<SphereData> generateSphereData(double arcRadius, double arcAngle, std::size_t sphereNum,
                                                      double sphereRadius);

public:
    PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius,
                           bool shouldNormalizeMassCentre = true)
            : PolysphereTraits(generateSphereData(arcRadius, arcAngle, sphereNum, sphereRadius), {0, 1, 0},
                               shouldNormalizeMassCentre)
    { }

    PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius,
                           std::unique_ptr<CentralInteraction> centralInteraction,
                           bool shouldNormalizeMassCentre = true)
            : PolysphereTraits(generateSphereData(arcRadius, arcAngle, sphereNum, sphereRadius),
                               std::move(centralInteraction), {0, 1, 0}, shouldNormalizeMassCentre)
    { }
};


#endif //RAMPACK_POLYSPHEREBANANATRAITS_H
