//
// Created by Piotr Kubala on 02/01/2021.
//

#ifndef RAMPACK_KMERTRAITS_H
#define RAMPACK_KMERTRAITS_H

#include "GenericPolysphereTraits.h"


/**
 * @brief A class representing linear k-polymer.
 * @details <p> The polymer lies on the Z axis (which consequently is its primary axis). Geometric center coincides with
 * the mass center (the endpoint spheres have opposite z coordinates). Apart from the named points inherited from
 * PolysphereTraits, the class specifies custom ones: "beg" and "end" for the spheres with centers lying on,
 * respectively, negative and positive Z half-axes.
 *
 * <p> The class is a thin decorator of PolysphereTraits, which means that arbitrary PolysphereShape species can be
 * added using addSpecies() method.
 */
class KMerTraits : public GenericPolysphereTraits {
private:
    static double calculateVolume(std::size_t sphereNum, double sphereRadius, double distance);

public:
    /**
     * @brief Generates the PolysphereShape of a k-mer, compatible with addSpecies().
     * @param sphereNum number of spheres in the k-mer. It must be &ge; 2
     * @param sphereRadius radius of the spheres. It must be positive
     * @param distance the distance between adjacent spheres' centers. It must be positive
     * @throws PreconditionException for a malformed shape (see the constraints of the arguments)
     */
    static PolysphereShape generateShape(std::size_t sphereNum, double sphereRadius, double distance);

    /**
     * @brief Creates the class with hard-core interactions and no initially registered species.
     */
    KMerTraits() = default;

    /**
     * @brief Creates the class with hard-core interactions and one species named `A` based on the given arguments,
     * which is set as a default species (setDefaultSpecies()). The arguments have the identical meaning as in
     * generateShape().
     */
    KMerTraits(std::size_t sphereNum, double sphereRadius, double distance)
            : GenericPolysphereTraits(generateShape(sphereNum, sphereRadius, distance))
    { }

    /**
     * @brief Creates the class with soft interactions @a centralInteraction and no initially registered species.
     */
    explicit KMerTraits(const std::shared_ptr<CentralInteraction> &centralInteraction)
            : GenericPolysphereTraits(centralInteraction)
    { }

    /**
     * @brief Creates the class with soft interactions @a centralInteraction and one species named `A` based on the
     * given arguments, which is set as a default species (setDefaultSpecies()). The arguments have the identical
     * meaning as in generateShape().
     */
    KMerTraits(std::size_t sphereNum, double sphereRadius, double distance,
               const std::shared_ptr<CentralInteraction> &centralInteraction)
            : GenericPolysphereTraits(generateShape(sphereNum, sphereRadius, distance), centralInteraction)
    { }

    /**
     * @brief Registers a new species named @a shapeName of a k-mer given by the rest of the arguments, whose meaning is
     * the same as in generateShape().
     */
    void addKMerShape(const std::string &shapeName, std::size_t sphereNum, double sphereRadius, double distance) {
        this->addSpecies(shapeName, KMerTraits::generateShape(sphereNum, sphereRadius, distance));
    }
};


#endif //RAMPACK_KMERTRAITS_H
