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
    static std::vector<SphereData> generateSphereData(std::size_t sphereNum, double smallSphereRadius,
                                                      double largeSphereRadius, double spherePenetration);

public:
    /**
     * @brief Constructs the class for hard interactions.
     * @param sphereNum number of all spheres
     * @param smallSphereRadius radius of the smallest (first) sphere
     * @param largeSphereRadius radius of the largest (last) sphere
     * @param spherePenetration how much spheres overlap (in particular 0 means tangent spheres)
     * @param shouldNormalizeMassCentre if @a true, particle origin will be placed in its mass centre
     */
    PolysphereWedgeTraits(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                          double spherePenetration, bool shouldNormalizeMassCentre = true)
            : PolysphereTraits(generateSphereData(sphereNum, smallSphereRadius, largeSphereRadius, spherePenetration),
                               {1, 0, 0}, {0, 1, 0}, shouldNormalizeMassCentre)
    { }

    /**
     * @brief Similar as PolysphereWedgeTraits::PolysphereWedgeTraits(std::size_t, double, double, double, bool), but
     * with soft central interaction given by @a centralInteraction.
     */
    PolysphereWedgeTraits(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                             double spherePenetration, std::unique_ptr<CentralInteraction> centralInteraction,
                             bool shouldNormalizeMassCentre = true)
            : PolysphereTraits(generateSphereData(sphereNum, smallSphereRadius, largeSphereRadius, spherePenetration),
                               std::move(centralInteraction), {1, 0, 0}, {0, 1, 0}, shouldNormalizeMassCentre)
    { }
};


#endif //RAMPACK_POLYSPHEREWEDGETRAITS_H
