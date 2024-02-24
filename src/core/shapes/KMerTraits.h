//
// Created by Piotr Kubala on 02/01/2021.
//

#ifndef RAMPACK_KMERTRAITS_H
#define RAMPACK_KMERTRAITS_H

#include "PolysphereTraits.h"


/**
 * @brief A class representing linear k-polymer.
 * @details The polymer lies on Z axis (which consequently is its primary axis). Secondary axis is X axis - formally
 * it is degenerate in XY plane, but was arbitrarily chosen to enable flip moves. Geometric centre coincides with
 * mass centre (endpoint spheres have opposite z coordinates). The class specifies custom named points "beg" and "end"
 * for first and last spheres, together with the ones inherited from PolysphereTraits.
 */
class KMerTraits : public PolysphereTraits {
private:
    static double caluclateVolume(std::size_t sphereNum, double sphereRadius, double distance);

public:
    static PolysphereShape generateShape(std::size_t sphereNum, double sphereRadius, double distance);

    KMerTraits() = default;

    /**
     * @brief A hard polymer of identical spheres.
     * @param sphereNum number of monomers
     * @param sphereRadius radius of each sphere
     * @param distance the distance between adjacent spheres' centres
     */
    KMerTraits(std::size_t sphereNum, double sphereRadius, double distance)
            : PolysphereTraits(generateShape(sphereNum, sphereRadius, distance))
    { }

    explicit KMerTraits(const std::shared_ptr<CentralInteraction> &centralInteraction)
            : PolysphereTraits(centralInteraction)
    { }

    /**
     * @brief Similar as KMerTraits::KMerTraits(std::size_t, double, double), but with soft central interaction given by
     * @a centralInteraction.
     */
    KMerTraits(std::size_t sphereNum, double sphereRadius, double distance,
               const std::shared_ptr<CentralInteraction> &centralInteraction)
            : PolysphereTraits(generateShape(sphereNum, sphereRadius, distance), centralInteraction)
    { }

    void addKMerShape(const std::string &shapeName, std::size_t sphereNum, double sphereRadius, double distance) {
        this->addPolysphereShape(shapeName, KMerTraits::generateShape(sphereNum, sphereRadius, distance));
    }
};


#endif //RAMPACK_KMERTRAITS_H
