//
// Created by Piotr Kubala on 26/04/2021.
//

#ifndef RAMPACK_POLYSPHEROCYLINDERBANANATRAITS_H
#define RAMPACK_POLYSPHEROCYLINDERBANANATRAITS_H

#include "PolyspherocylinderTraits.h"

class PolyspherocylinderBananaTraits : public PolyspherocylinderTraits {
private:
    static std::vector<PolyspherocylinderTraits::SpherocylinderData>
    generateSpherocylinderData(double arcRadius, double arcAngle, std::size_t segmentsNum,
                               double radius, std::size_t subdivisions);

public:
    PolyspherocylinderBananaTraits(double arcRadius, double arcAngle, std::size_t segmentsNum, double radius,
                                   std::size_t subdivisions = 1, bool shouldNormalizeMassCentre = true)
            : PolyspherocylinderTraits(generateSpherocylinderData(arcRadius, arcAngle, segmentsNum,
                                                                  radius, subdivisions),
                                       {0, 1, 0}, shouldNormalizeMassCentre)
    { }
};


#endif //RAMPACK_POLYSPHEROCYLINDERBANANATRAITS_H
