//
// Created by pkua on 01.03.2022.
//

#ifndef RAMPACK_POLYSPHERELOLLIPOPTRAITS_H
#define RAMPACK_POLYSPHERELOLLIPOPTRAITS_H

#include "PolysphereTraits.h"

/**
 * @brief A class representing linear sphere polymer capped with one larger sphere (lollipop-shaped).
 * @details The molecule is spanned on x axis and, if not normalized (@a shouldNormalizeMassCentre parameter in
 * constructors), the center lies in the first small sphere's center.
 */
class PolysphereLollipopTraits : public PolysphereTraits {
private:
    static std::vector<SphereData> generateSphereData(std::size_t sphereNum, double smallSphereRadius,
                                                      double largeSphereRadius, double smallSpherePenetration,
                                                      double largeSpherePenetration);

public:
    /**
     * @brief Constructs the class for hard interactions.
     * @param sphereNum number of all spheres - there is @a sphereNum - 1 small spheres and a single large one
     * @param smallSphereRadius radius of small spheres
     * @param largeSphereRadius radius of a large sphere
     * @param smallSpherePenetration how much small spheres overlap (in particular 0 means tangent spheres)
     * @param largeSpherePenetration hom much the large sphere overlaps with the last small
     * @param shouldNormalizeMassCentre if @a true, particle origin will be placed in its mass centre
     */
    PolysphereLollipopTraits(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                             double smallSpherePenetration, double largeSpherePenetration,
                             bool shouldNormalizeMassCentre = true)
            : PolysphereTraits(generateSphereData(sphereNum, smallSphereRadius, largeSphereRadius,
                                                  smallSpherePenetration, largeSpherePenetration),
                               {1, 0, 0}, {0, 1, 0}, shouldNormalizeMassCentre)
    { }

    /**
     * @brief Similar as
     * PolysphereLollipopTraits::PolysphereLollipopTraits(std::size_t, double, double, double, double, bool), but with
     * soft central interaction given by @a centralInteraction.
     */
    PolysphereLollipopTraits(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                             double smallSpherePenetration, double largeSpherePenetration,
                             std::unique_ptr<CentralInteraction> centralInteraction,
                             bool shouldNormalizeMassCentre = true)
            : PolysphereTraits(generateSphereData(sphereNum, smallSphereRadius, largeSphereRadius,
                                                  smallSpherePenetration, largeSpherePenetration),
                               std::move(centralInteraction), {1, 0, 0}, {0, 1, 0}, shouldNormalizeMassCentre)
    { }
};



#endif //RAMPACK_POLYSPHERELOLLIPOPTRAITS_H
