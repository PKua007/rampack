//
// Created by pkua on 09.03.2022.
//

#ifndef RAMPACK_POLYSPHEREWEDGETRAITS_H
#define RAMPACK_POLYSPHEREWEDGETRAITS_H

#include "PolysphereTraits.h"


namespace legacy {
    /**
     * @brief A legacy (pre 0.2.0) version of class representing linear sphere polymer where radii of spheres grow
     * linearly.
     * @details The molecule is spanned on x axis and centered in its mass centre. Primary axis is naturally x axis
     * (positive, towards the largest sphere). Secondary axis is y axis - formally it is degenerate in yz plane, but was
     * arbitrarily chosen to enable flip moves. Geometric centre lies in the centre of a bounding box (it coincides with
     * the mass centre only if all spheres have the same radius). The class specifies custom named points "ss" and "sl"
     * for first (small) and last (large) spheres, together with once inherited from PolysphereTraits.
     * @sa ::PolysphereWedgeTraits
     */
    class PolysphereWedgeTraits : public PolysphereTraits {
    private:
        static PolysphereGeometry generateGeometry(std::size_t sphereNum, double smallSphereRadius,
                                                   double largeSphereRadius, double spherePenetration);

    public:
        /**
         * @brief Constructs the class for hard interactions.
         * @param sphereNum number of all spheres
         * @param smallSphereRadius radius of the smallest (first) sphere
         * @param largeSphereRadius radius of the largest (last) sphere
         * @param spherePenetration how much spheres overlap (in particular 0 means tangent spheres)
         */
        PolysphereWedgeTraits(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                              double spherePenetration)
                : PolysphereTraits(generateGeometry(sphereNum, smallSphereRadius, largeSphereRadius, spherePenetration))
        { }

        /**
         * @brief Similar as PolysphereWedgeTraits::PolysphereWedgeTraits(std::size_t, double, double, double), but
         * with soft central interaction given by @a centralInteraction.
         */
        PolysphereWedgeTraits(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                              double spherePenetration, std::shared_ptr<CentralInteraction> centralInteraction)
                : PolysphereTraits(generateGeometry(sphereNum, smallSphereRadius, largeSphereRadius, spherePenetration),
                                   std::move(centralInteraction))
        { }
    };
}


/**
 * @brief A class representing linear sphere polymer where radii of spheres grow linearly.
 * @details The molecule is spanned on z axis and centered in the center of the bounding box in order to minimize the
 * circumsphere radius. Primary axis is naturally positive z axis. Secondary axis is x axis - formally it is degenerate
 * in xz plane, but was arbitrarily chosen to enable flip moves. The class specifies custom named points "beg" and "end"
 * for bottom and top spheres, together with once inherited from PolysphereTraits. Mass centre "cm" named point is
 * defined only if @a spherePenetration is zero.
 * @sa legacy::PolysphereWedgeTraits
 */
class PolysphereWedgeTraits : public PolysphereTraits {
private:
    static PolysphereGeometry generateGeometry(std::size_t sphereNum, double bottomSphereRadius, double topSphereRadius,
                                               double spherePenetration);

public:
    /**
     * @brief Constructs the class for hard interactions.
     * @param sphereNum number of all spheres
     * @param topSphereRadius radius of the bottom sphere
     * @param bottomSphereRadius radius of the top sphere
     * @param spherePenetration how much spheres overlap (in particular 0 means tangent spheres)
     */
    PolysphereWedgeTraits(std::size_t sphereNum, double bottomSphereRadius, double topSphereRadius,
                          double spherePenetration)
            : PolysphereTraits(generateGeometry(sphereNum, bottomSphereRadius, topSphereRadius, spherePenetration))
    { }

    /**
     * @brief Similar as PolysphereWedgeTraits::PolysphereWedgeTraits(std::size_t, double, double, double), but with
     * soft central interaction given by @a centralInteraction.
     */
    PolysphereWedgeTraits(std::size_t sphereNum, double bottomSphereRadius, double topSphereRadius,
                          double spherePenetration, std::shared_ptr<CentralInteraction> centralInteraction)
            : PolysphereTraits(generateGeometry(sphereNum, bottomSphereRadius, topSphereRadius, spherePenetration),
                               std::move(centralInteraction))
    { }
};


#endif //RAMPACK_POLYSPHEREWEDGETRAITS_H
