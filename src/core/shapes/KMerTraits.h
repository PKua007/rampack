//
// Created by Piotr Kubala on 02/01/2021.
//

#ifndef RAMPACK_KMERTRAITS_H
#define RAMPACK_KMERTRAITS_H

#include "PolysphereTraits.h"

/**
 * @brief A class representing linear k-polymer.
 * @details The polymer lies on X axis (which consequently is its primary axis). Secondary axis is Y axis - formally
 * it is degenerate in YZ plane, but was arbitrarily chosen to enable flip moves. Geometric centre coincides with
 * mass centre (endpoint spheres have opposite x coordinates). The class specifies custom named points "sbeg" and "send"
 * for first and last spheres, together with once inherited from PolysphereTraits.
 */
class KMerTraits : public PolysphereTraits {
private:
    static PolysphereGeometry generateGeometry(std::size_t sphereNum, double sphereRadius, double distance);

public:
    /**
     * @brief A hard polymer of identical spheres.
     * @param sphereNum number of monomers
     * @param sphereRadius radius of each sphere
     * @param distance the distance between adjacent spheres' centres
     */
    KMerTraits(std::size_t sphereNum, double sphereRadius, double distance)
            : PolysphereTraits(generateGeometry(sphereNum, sphereRadius, distance))
    { }

    /**
     * @brief Similar as KMerTraits::KMerTraits(std::size_t, double, double), but with soft central interaction given by
     * @a centralInteraction.
     */
    KMerTraits(std::size_t sphereNum, double sphereRadius, double distance,
               std::unique_ptr<CentralInteraction> centralInteraction)
            : PolysphereTraits(generateGeometry(sphereNum, sphereRadius, distance), std::move(centralInteraction))
    { }
};


#endif //RAMPACK_KMERTRAITS_H
