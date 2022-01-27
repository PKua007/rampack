//
// Created by Piotr Kubala on 26/04/2021.
//

#ifndef RAMPACK_POLYSPHEROCYLINDERBANANATRAITS_H
#define RAMPACK_POLYSPHEROCYLINDERBANANATRAITS_H

#include "PolyspherocylinderTraits.h"

/**
 * @brief A bent shaped molecule build of hard spherocylinders placed on an arc.
 */
class PolyspherocylinderBananaTraits : public PolyspherocylinderTraits {
private:
    static std::vector<PolyspherocylinderTraits::SpherocylinderData>
    generateSpherocylinderData(double arcRadius, double arcAngle, std::size_t segmentsNum,
                               double radius, std::size_t subdivisions);

public:
    /**
     * @brief Constructor with analogous parameters as for PolysphereBananaTraits, but instead the arc is divided into
     * @a segmentsNum segments, on which the spherocylinders radius are built.
     * @param arcRadius see PolysphereBananaTraits
     * @param arcAngle see PolysphereBananaTraits
     * @param segmentsNum number of segments to divide the arc into
     * @param radius radius (half-length) of spherocylinder
     * @param subdivisions additional divisions of spherocylinders. It does not change the shape, but decreases the
     * interaction range of a single interaction centre, so it may increase the speed
     * @param shouldNormalizeMassCentre see PolysphereBananaTraits
     */
    PolyspherocylinderBananaTraits(double arcRadius, double arcAngle, std::size_t segmentsNum, double radius,
                                   std::size_t subdivisions = 1, bool shouldNormalizeMassCentre = true)
            : PolyspherocylinderTraits(generateSpherocylinderData(arcRadius, arcAngle, segmentsNum,
                                                                  radius, subdivisions),
                                       {0, 1, 0}, shouldNormalizeMassCentre)
    { }
};


#endif //RAMPACK_POLYSPHEROCYLINDERBANANATRAITS_H
