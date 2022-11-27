//
// Created by Piotr Kubala on 26/04/2021.
//

#ifndef RAMPACK_POLYSPHEROCYLINDERBANANATRAITS_H
#define RAMPACK_POLYSPHEROCYLINDERBANANATRAITS_H

#include "PolyspherocylinderTraits.h"

/**
 * @brief A bent shaped molecule build of hard spherocylinders placed on an arc. The construction is analogous to
 * PolysphereBananaTraits.
 * @details The class specifies custom named points "sbeg" and "send" for endpoints of the banana, together with the
 * ones inherited from PolysphereTraits.
 */
class PolyspherocylinderBananaTraits : public PolyspherocylinderTraits {
private:
    static PolyspherocylinderTraits::PolyspherocylinderGeometry generateGeometry(double arcRadius, double arcAngle,
                                                                                 std::size_t segmentsNum, double radius,
                                                                                 std::size_t subdivisions);

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
     * @param normalizeMassCentre if @a true, mass centre will be normalized as per
     * PolyspherocylinderTraits::PolyspherocylinderGeometry::normalizeMassCentre()
     */
    PolyspherocylinderBananaTraits(double arcRadius, double arcAngle, std::size_t segmentsNum, double radius,
                                   std::size_t subdivisions = 1)
            : PolyspherocylinderTraits(generateGeometry(arcRadius, arcAngle, segmentsNum, radius, subdivisions))
    { }
};


#endif //RAMPACK_POLYSPHEROCYLINDERBANANATRAITS_H
