//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_POLYSPHEREBANANATRAITS_H
#define RAMPACK_POLYSPHEREBANANATRAITS_H

#include "PolysphereTraits.h"

/**
 * @brief A banana-shaped polymer of spheres.
 */
class PolysphereBananaTraits : public PolysphereTraits {
private:
    static std::vector<SphereData> generateSphereData(double arcRadius, double arcAngle, std::size_t sphereNum,
                                                      double sphereRadius);

public:
    /**
     * @brief The hard polymer built by placing spheres on an arc. The sphere can be tangent or overlapping, depending
     * on the parameters.
     * @details The arc lies in the xy plane with x = 0, y = 0 origin, symmetric w.r.t. x axis and lying in negative
     * x half-plane. The primary (molecular) axis is thus y axis.
     * @param arcRadius the radius of the arc
     * @param arcAngle the arc angle
     * @param sphereNum number of spheres to be equidistantly placed on the arc. First and last sphere centres are arc
     * endpoints
     * @param sphereRadius the radius of each sphere
     * @param shouldNormalizeMassCentre if true, the mass centre will be moved to the origin. If false, no mass centre
     * normalization will be done
     */
    PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius,
                           bool shouldNormalizeMassCentre = true)
            : PolysphereTraits(generateSphereData(arcRadius, arcAngle, sphereNum, sphereRadius), {0, 1, 0}, {-1, 0, 0},
                               shouldNormalizeMassCentre)
    { }

    /**
     * @brief Similar as PolysphereBananaTraits::PolysphereBananaTraits(double, double, std::size_t, double, bool), but
     * for soft central interactions given by @a centralInteraction.
     */
    PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius,
                           std::unique_ptr<CentralInteraction> centralInteraction,
                           bool shouldNormalizeMassCentre = true)
            : PolysphereTraits(generateSphereData(arcRadius, arcAngle, sphereNum, sphereRadius),
                               std::move(centralInteraction), {0, 1, 0}, {-1, 0, 0}, shouldNormalizeMassCentre)
    { }
};


#endif //RAMPACK_POLYSPHEREBANANATRAITS_H
