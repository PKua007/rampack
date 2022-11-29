//
// Created by pkua on 09.03.2022.
//

#ifndef RAMPACK_POLYSPHEREWEDGETRAITS_H
#define RAMPACK_POLYSPHEREWEDGETRAITS_H

#include "PolysphereTraits.h"


namespace legacy {
    /**
     * @brief A class representing linear sphere polymer where radii of spheres grow linearly.
     * @details The molecule is spanned on x axis and, if not normalized (@a shouldNormalizeMassCentre parameter in
     * constructors), the center lies in the first sphere's center. Primary axis is naturally x axis (positive,
     * towards the largest sphere). Secondary axis is Y axis - formally it is degenerate in YZ plane, but was arbitrarily
     * chosen to enable flip moves. Geometric centre lies in the centre of a bounding box (it coincides with the mass centre
     * only if all spheres have the same radius and mass centre is normalized). The class specifies custom named points "ss"
     * and "sl" for first (small) and last (large) spheres, together with once inherited from PolysphereTraits.
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
         * @param normalizeMassCentre if @a true, particle origin will be placed in its mass centre
         */
        PolysphereWedgeTraits(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                              double spherePenetration)
                : PolysphereTraits(generateGeometry(sphereNum, smallSphereRadius, largeSphereRadius, spherePenetration))
        { }

        /**
         * @brief Similar as PolysphereWedgeTraits::PolysphereWedgeTraits(std::size_t, double, double, double, bool), but
         * with soft central interaction given by @a centralInteraction.
         */
        PolysphereWedgeTraits(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                              double spherePenetration, std::unique_ptr<CentralInteraction> centralInteraction)
                : PolysphereTraits(generateGeometry(sphereNum, smallSphereRadius, largeSphereRadius, spherePenetration),
                                   std::move(centralInteraction))
        { }
    };
}


/**
 * @brief A class representing linear sphere polymer where radii of spheres grow linearly.
 * @details The molecule is spanned on x axis and, if not normalized (@a shouldNormalizeMassCentre parameter in
 * constructors), the center lies in the first sphere's center. Primary axis is naturally x axis (positive,
 * towards the largest sphere). Secondary axis is Y axis - formally it is degenerate in YZ plane, but was arbitrarily
 * chosen to enable flip moves. Geometric centre lies in the centre of a bounding box (it coincides with the mass centre
 * only if all spheres have the same radius and mass centre is normalized). The class specifies custom named points "ss"
 * and "sl" for first (small) and last (large) spheres, together with once inherited from PolysphereTraits.
 */
class PolysphereWedgeTraits : public PolysphereTraits {
private:
    static PolysphereGeometry generateGeometry(std::size_t sphereNum, double bottomSphereRadius, double topSphereRadius,
                                               double spherePenetration);

public:
    /**
     * @brief Constructs the class for hard interactions.
     * @param sphereNum number of all spheres
     * @param topSphereRadius radius of the smallest (first) sphere
     * @param bottomSphereRadius radius of the largest (last) sphere
     * @param spherePenetration how much spheres overlap (in particular 0 means tangent spheres)
     * @param normalizeMassCentre if @a true, particle origin will be placed in its mass centre
     */
    PolysphereWedgeTraits(std::size_t sphereNum, double bottomSphereRadius, double topSphereRadius,
                          double spherePenetration)
            : PolysphereTraits(generateGeometry(sphereNum, bottomSphereRadius, topSphereRadius, spherePenetration))
    { }


    /**
     * @brief Similar as PolysphereWedgeTraits::PolysphereWedgeTraits(std::size_t, double, double, double, bool), but
     * with soft central interaction given by @a centralInteraction.
     */
    PolysphereWedgeTraits(std::size_t sphereNum, double bottomSphereRadius, double topSphereRadius,
                          double spherePenetration, std::unique_ptr<CentralInteraction> centralInteraction)
            : PolysphereTraits(generateGeometry(sphereNum, bottomSphereRadius, topSphereRadius, spherePenetration),
                               std::move(centralInteraction))
    { }
};


#endif //RAMPACK_POLYSPHEREWEDGETRAITS_H
