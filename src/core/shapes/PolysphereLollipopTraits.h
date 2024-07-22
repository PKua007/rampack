//
// Created by pkua on 01.03.2022.
//

#ifndef RAMPACK_POLYSPHERELOLLIPOPTRAITS_H
#define RAMPACK_POLYSPHERELOLLIPOPTRAITS_H

#include "GenericPolysphereTraits.h"


namespace legacy {
    /**
     * @brief A legacy (pre 0.2.0) version of class representing linear sphere polymer capped with one larger sphere
     * (lollipop-shaped).
     * @details The molecule is spanned on x axis and centered in its mass centre. Primary axis is naturally x axis
     * (positive, towards the large sphere). Secondary axis is y axis - formally it is degenerate in yz plane, but was
     * arbitrarily chosen to enable flip moves. Geometric centre lies in the centre of a bounding box (it coincides with
     * the mass centre only if all spheres have the same radii). The class specifies custom named points "ss" and "sl"
     * for first (small) and last (large) spheres, together with the ones inherited from PolysphereTraits.
     * @sa ::PolysphereLollipopTraits
     */
    class PolysphereLollipopTraits : public GenericPolysphereTraits {
    private:
        static PolysphereShape generateShape(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                                             double smallSpherePenetration, double largeSpherePenetration);

    public:
        /**
         * @brief Constructs the class for hard interactions.
         * @param sphereNum number of all spheres - there is @a sphereNum - 1 small spheres and a single large one
         * @param smallSphereRadius radius of small spheres
         * @param largeSphereRadius radius of a large sphere
         * @param smallSpherePenetration how much small spheres overlap (in particular 0 means tangent spheres)
         * @param largeSpherePenetration hom much the large sphere overlaps with the last small
         */
        PolysphereLollipopTraits(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                                 double smallSpherePenetration, double largeSpherePenetration)
                : GenericPolysphereTraits(generateShape(sphereNum, smallSphereRadius, largeSphereRadius,
                                                 smallSpherePenetration, largeSpherePenetration))
        { }

        /**
         * @brief Similar as
         * PolysphereLollipopTraits::PolysphereLollipopTraits(std::size_t, double, double, double, double), but with
         * soft central interaction given by @a centralInteraction.
         */
        PolysphereLollipopTraits(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                                 double smallSpherePenetration, double largeSpherePenetration,
                                 const std::shared_ptr<CentralInteraction> &centralInteraction)
                : GenericPolysphereTraits(generateShape(sphereNum, smallSphereRadius, largeSphereRadius,
                                                 smallSpherePenetration, largeSpherePenetration),
                                   centralInteraction)
        { }
    };
}


/**
 * @brief A class representing linear sphere polymer capped with one sphere of a different size (a lollipop).
 * @details <p> The molecule is spanned on the Z axis and centered in the midpoint of a bounding box to minimize the
 * circumsphere radius. The primary axis is naturally the Z axis (positive, towards the tip). The class specifies
 * custom named points "ss" and "st" for the first (the end of lollipop's stick) and last (the tip of the lollipop)
 * spheres, together with the ones inherited from PolysphereTraits. Named point "cm" denoting the mass center is defined
 * only if the spheres do not overlap.
 *
 * <p> The class is a thin decorator of PolysphereTraits, which means that arbitrary PolysphereShape species can be
 * added using addSpecies() method.
 * @sa legacy::PolysphereLollipopTraits
 */
class PolysphereLollipopTraits : public GenericPolysphereTraits {
private:
    static double calculateVolume(const std::vector<SphereData> &sphereData, double stickSpherePenetration,
                                  double tipSpherePenetration);

public:
    /**
     * @brief Generates the PolysphereShape of a lollipop, compatible with addSpecies().
     * @param sphereNum number of all spheres - there is @a sphereNum - 1 small spheres and a single large one. It must
     * be &ge; 2
     * @param stickSphereRadius radius of the spheres building the lollipop's stick. It must be positive
     * @param tipSphereRadius radius of the lollipop's tip sphere. It must be positive
     * @param stickSpherePenetration how much stick spheres overlap (in particular, 0 yields tangent spheres). It must
     * lie in the range 0 &le; stickSpherePenetration < 2 &middot; @a stickSphereRadius
     * @param tipSpherePenetration how much the tip sphere overlaps with the sphere at the adjacent endpoint of the
     * lollipop's stick. It must lie in the range
     * 0 &le; tipSpherePenetration < 2 &middot; min(@a stickSphereRadius, @a tipSphereRadius)
     * @throws PreconditionException if the shape is malformed (see the constraints of the parameters).
     */
    static PolysphereShape generateShape(std::size_t sphereNum, double stickSphereRadius, double tipSphereRadius,
                                         double stickSpherePenetration, double tipSpherePenetration);

    /**
     * @brief Creates the class with hard-core interactions and no initially registered species.
     */
    PolysphereLollipopTraits() = default;

    /**
     * @brief Creates the class with hard-core interactions and one species named `A` based on the given arguments,
     * which is set as a default species (setDefaultSpecies()). The arguments have the identical meaning as in
     * generateShape().
     */
    PolysphereLollipopTraits(std::size_t sphereNum, double stickSphereRadius, double tipSphereRadius,
                             double stickSpherePenetration, double tipSpherePenetration)
            : GenericPolysphereTraits(generateShape(sphereNum, stickSphereRadius, tipSphereRadius,
                                             stickSpherePenetration, tipSpherePenetration))
    { }

    /**
     * @brief Creates the class with soft interactions @a centralInteraction and no initially registered species.
     */
    explicit PolysphereLollipopTraits(const std::shared_ptr<CentralInteraction> &centralInteraction)
            : GenericPolysphereTraits(centralInteraction)
    { }

    /**
     * @brief Creates the class with soft interactions @a centralInteraction and one species named `A` based on the
     * given arguments, which is set as a default species (setDefaultSpecies()). The arguments have the identical
     * meaning as in generateShape().
     */
    PolysphereLollipopTraits(std::size_t sphereNum, double stickSphereRadius, double tipSphereRadius,
                             double stickSpherePenetration, double tipSpherePenetration,
                             const std::shared_ptr<CentralInteraction> &centralInteraction)
            : GenericPolysphereTraits(generateShape(sphereNum, stickSphereRadius, tipSphereRadius,
                                             stickSpherePenetration, tipSpherePenetration),
                               centralInteraction)
    { }

    /**
     * @brief Registers a new species named @a shapeName of a lollipop given by the rest of the arguments, whose meaning
     * is the same as in generateShape().
     */
    void addLollipopShape(const std::string &shapeName, std::size_t sphereNum, double stickSphereRadius,
                          double tipSphereRadius, double stickSpherePenetration, double tipSpherePenetration)
    {
        auto shape = PolysphereLollipopTraits::generateShape(sphereNum, stickSphereRadius, tipSphereRadius,
                                                             stickSpherePenetration, tipSpherePenetration);
        this->addSpecies(shapeName, shape);
    }
};



#endif //RAMPACK_POLYSPHERELOLLIPOPTRAITS_H
