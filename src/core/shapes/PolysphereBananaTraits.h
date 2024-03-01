//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_POLYSPHEREBANANATRAITS_H
#define RAMPACK_POLYSPHEREBANANATRAITS_H

#include "PolysphereTraits.h"


namespace legacy {
    /**
     * @brief The legacy version (pre 0.2.0) of the banana-shaped hard polymer built by placing spheres on an arc. The
     * sphere can be tangent or overlapping, depending on the parameters.
     * @details The arc lies in the xy plane with x = 0, y = 0 origin, symmetric w.r.t. x axis and lying in negative
     * x half-plane. The primary (molecular) axis is thus y axis, while secondary (polarization) axis is negative x
     * axis. Geometric origin is placed in the mass centre. The class specifies custom named points "sbeg" and "send"
     * for first and last spheres, together with the ones inherited from PolysphereTraits.
     * @sa ::PolysphereBananaTraits
     */
    class PolysphereBananaTraits : public PolysphereTraits {
    private:
        static PolysphereShape generateShape(double arcRadius, double arcAngle, std::size_t sphereNum,
                                             double sphereRadius);

    public:
        /**
         * @brief Constructs the shape.
         * @param arcRadius the radius of the arc
         * @param arcAngle the arc angle
         * @param sphereNum number of spheres to be equidistantly placed on the arc. First and last sphere centres are arc
         * endpoints
         * @param sphereRadius the radius of each sphere
         */
        PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius)
                : PolysphereTraits(generateShape(arcRadius, arcAngle, sphereNum, sphereRadius))
        { }

        /**
         * @brief Similar as PolysphereBananaTraits::PolysphereBananaTraits(double, double, std::size_t, double), but
         * for soft central interactions given by @a centralInteraction.
         */
        PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius,
                               const std::shared_ptr<CentralInteraction> &centralInteraction)
                : PolysphereTraits(generateShape(arcRadius, arcAngle, sphereNum, sphereRadius), centralInteraction)
        { }
    };
}


/**
 * @brief The banana-shaped hard polymer built by placing spheres on an arc. The sphere can be tangent or overlapping,
 * depending on the parameters.
 * @details The arc lies in the xz, symmetric w.r.t. x axis and it is bent towards negative x half-space. For
 * @a arcRadius smaller than \f$\pi\f$, the origin in the middle of the line joining endpoints, while for larger
 * @a arcRadius it coincides with arc's midpoint (it is defined in such a way to minimize circumsphere radius). The
 * primary (molecular) axis is z axis, while secondary axis is negative x axis. The class specifies custom named points
 * "beg" and "end" for first and last spheres, together with the ones inherited from PolysphereTraits. Mass centre "cm"
 * named point is defined only in the spheres don't overlap.
 * @sa legacy::PolysphereBananaTraits
 */
class PolysphereBananaTraits : public PolysphereTraits {
private:
    static double calculateVolume(const std::vector<SphereData> &sphereData, double arcAngle);
    static void addMassCentre(PolysphereShape &shape);

public:
    static PolysphereShape generateShape(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius);

    PolysphereBananaTraits() = default;

    /**
     * @brief Constructs the shape.
     * @param arcRadius the radius of the arc
     * @param arcAngle the arc angle
     * @param sphereNum number of spheres to be equidistantly placed on the arc. First and last sphere centres are arc
     * endpoints
     * @param sphereRadius the radius of each sphere
     */
    PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius)
            : PolysphereTraits(generateShape(arcRadius, arcAngle, sphereNum, sphereRadius))
    { }

    explicit PolysphereBananaTraits(const std::shared_ptr<CentralInteraction> &centralInteraction)
            : PolysphereTraits(centralInteraction)
    { }

    /**
     * @brief Similar as PolysphereBananaTraits::PolysphereBananaTraits(double, double, std::size_t, double), but for
     * soft central interactions given by @a centralInteraction.
     */
    PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius,
                           const std::shared_ptr<CentralInteraction> &centralInteraction)
            : PolysphereTraits(generateShape(arcRadius, arcAngle, sphereNum, sphereRadius), centralInteraction)
    { }

    void addBananaShape(const std::string &shapeName, double arcRadius, double arcAngle, std::size_t sphereNum,
                        double sphereRadius)
    {
        this->addSpecies(
                shapeName, PolysphereBananaTraits::generateShape(arcRadius, arcAngle, sphereNum, sphereRadius)
        );
    }
};


#endif //RAMPACK_POLYSPHEREBANANATRAITS_H
