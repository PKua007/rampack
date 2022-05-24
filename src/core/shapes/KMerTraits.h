//
// Created by Piotr Kubala on 02/01/2021.
//

#ifndef RAMPACK_KMERTRAITS_H
#define RAMPACK_KMERTRAITS_H

#include "PolysphereTraits.h"

/**
 * @brief A class representing linear k-polymer.
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
