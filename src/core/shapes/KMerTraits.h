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
    KMerTraits(std::size_t sphereNum, double sphereRadius, double distance)
            : PolysphereTraits(generateSphereData(sphereNum, sphereRadius, distance))
    { }

    KMerTraits(std::size_t sphereNum, double sphereRadius, double distance,
               std::unique_ptr<CentralInteraction> centralInteraction)
            : PolysphereTraits(generateSphereData(sphereNum, sphereRadius, distance),
                               std::move(centralInteraction))
    { }
};


#endif //RAMPACK_KMERTRAITS_H
