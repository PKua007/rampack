//
// Created by Piotr Kubala on 02/01/2021.
//

#ifndef RAMPACK_KMERTRAITS_H
#define RAMPACK_KMERTRAITS_H

#include "PolysphereTraits.h"

class KMerTraits : public PolysphereTraits {
private:
    static std::vector<SphereData> generateSphereData(std::size_t sphereNum, double sphereRadius, double distance);

public:
    KMerTraits(std::size_t sphereNum, double sphereRadius, double distance, bool shouldNormalizeMassCentre = true)
            : PolysphereTraits(generateSphereData(sphereNum, sphereRadius, distance), {1, 0, 0},
                               shouldNormalizeMassCentre)
    { }

    KMerTraits(std::size_t sphereNum, double sphereRadius, double distance,
               std::unique_ptr<CentralInteraction> centralInteraction, bool shouldNormalizeMassCentre = true)
            : PolysphereTraits(generateSphereData(sphereNum, sphereRadius, distance),
                               std::move(centralInteraction), {1, 0, 0}, shouldNormalizeMassCentre)
    { }
};


#endif //RAMPACK_KMERTRAITS_H
