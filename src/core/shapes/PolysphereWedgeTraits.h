//
// Created by pkua on 09.03.2022.
//

#ifndef RAMPACK_POLYSPHEREWEDGETRAITS_H
#define RAMPACK_POLYSPHEREWEDGETRAITS_H

#include "PolysphereTraits.h"


/**
 * @brief A class representing linear sphere polymer where radii of spheres grow linearly.
 * @details The molecule is spanned on x axis and, if not normalized (@a shouldNormalizeMassCentre parameter in
 * constructors), the center lies in the first sphere's center.
 */
class PolysphereWedgeTraits : public PolysphereTraits {
private:
    static PolysphereGeometry generateGeometry(std::size_t sphereNum, double smallSphereRadius,
                                               double largeSphereRadius, double spherePenetration,
                                               bool normalizeMassCentre);

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
                          double spherePenetration, bool normalizeMassCentre = true)
            : PolysphereTraits(generateGeometry(sphereNum, smallSphereRadius, largeSphereRadius, spherePenetration,
                                                normalizeMassCentre))
    { }

    /**
     * @brief Similar as PolysphereWedgeTraits::PolysphereWedgeTraits(std::size_t, double, double, double, bool), but
     * with soft central interaction given by @a centralInteraction.
     */
    PolysphereWedgeTraits(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                             double spherePenetration, std::unique_ptr<CentralInteraction> centralInteraction,
                             bool normalizeMassCentre = true)
            : PolysphereTraits(generateGeometry(sphereNum, smallSphereRadius, largeSphereRadius, spherePenetration,
                                                normalizeMassCentre),
                               std::move(centralInteraction))
    { }
};


#endif //RAMPACK_POLYSPHEREWEDGETRAITS_H
