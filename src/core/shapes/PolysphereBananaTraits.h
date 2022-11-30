//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_POLYSPHEREBANANATRAITS_H
#define RAMPACK_POLYSPHEREBANANATRAITS_H

#include "PolysphereTraits.h"


namespace legacy {
    /**
     * @brief The banana-shaped hard polymer built by placing spheres on an arc. The sphere can be tangent or overlapping,
     * depending on the parameters.
     * @details The arc lies in the xy plane with x = 0, y = 0 origin, symmetric w.r.t. x axis and lying in negative
     * x half-plane. The primary (molecular) axis is thus y axis, while secondary (polarization) axis is negative x
     * axis. Geometric origin is placed in the mass centre. The class specifies custom named points "sbeg" and "send"
     * for first and last spheres, together with the ones inherited from PolysphereTraits.
     */
    class PolysphereBananaTraits : public PolysphereTraits {
    private:
        static PolysphereGeometry generateGeometry(double arcRadius, double arcAngle, std::size_t sphereNum,
                                                   double sphereRadius);

    public:
        /**
         * @brief Constructs the shape.
         * @param arcRadius the radius of the arc
         * @param arcAngle the arc angle
         * @param sphereNum number of spheres to be equidistantly placed on the arc. First and last sphere centres are arc
         * endpoints
         * @param sphereRadius the radius of each sphere
         * PolysphereTraits::PolysphereGeometry::normalizeMassCentre()
         */
        PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius)
                : PolysphereTraits(generateGeometry(arcRadius, arcAngle, sphereNum, sphereRadius))
        { }

        /**
         * @brief Similar as PolysphereBananaTraits::PolysphereBananaTraits(double, double, std::size_t, double, bool), but
         * for soft central interactions given by @a centralInteraction.
         */
        PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius,
                               std::unique_ptr<CentralInteraction> centralInteraction)
                : PolysphereTraits(generateGeometry(arcRadius, arcAngle, sphereNum, sphereRadius),
                                   std::move(centralInteraction))
        { }
    };
}


/**
 * @brief The banana-shaped hard polymer built by placing spheres on an arc. The sphere can be tangent or overlapping,
 * depending on the parameters.
 * @details The arc lies in the xy plane with x = 0, y = 0 origin, symmetric w.r.t. x axis and lying in negative
 * x half-plane. The primary (molecular) axis is thus y axis, while secondary (polarization) axis is negative x
 * axis. Geometric origin is placed in the mass centre. The class specifies custom named points "sbeg" and "send"
 * for first and last spheres, together with the ones inherited from PolysphereTraits.
 */
class PolysphereBananaTraits : public PolysphereTraits {
private:
    static PolysphereGeometry generateGeometry(double arcRadius, double arcAngle, std::size_t sphereNum,
                                               double sphereRadius);
    static void addMassCentre(PolysphereGeometry &geometry);

public:
    /**
     * @brief Constructs the shape.
     * @param arcRadius the radius of the arc
     * @param arcAngle the arc angle
     * @param sphereNum number of spheres to be equidistantly placed on the arc. First and last sphere centres are arc
     * endpoints
     * @param sphereRadius the radius of each sphere
     * @param normalizeMassCentre if @a true, mass centre will be normalized as per
     * PolysphereTraits::PolysphereGeometry::normalizeMassCentre()
     */
    PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius)
            : PolysphereTraits(generateGeometry(arcRadius, arcAngle, sphereNum, sphereRadius))
    { }

    /**
     * @brief Similar as PolysphereBananaTraits::PolysphereBananaTraits(double, double, std::size_t, double, bool), but
     * for soft central interactions given by @a centralInteraction.
     */
    PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius,
                           std::unique_ptr<CentralInteraction> centralInteraction)
            : PolysphereTraits(generateGeometry(arcRadius, arcAngle, sphereNum, sphereRadius),
                               std::move(centralInteraction))
    { }
};


#endif //RAMPACK_POLYSPHEREBANANATRAITS_H
