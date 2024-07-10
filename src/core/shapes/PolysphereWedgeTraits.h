//
// Created by pkua on 09.03.2022.
//

#ifndef RAMPACK_POLYSPHEREWEDGETRAITS_H
#define RAMPACK_POLYSPHEREWEDGETRAITS_H

#include "PolysphereTraits.h"


namespace legacy {
    /**
     * @brief A legacy (pre 0.2.0) version of class representing linear sphere polymer where radii of spheres grow
     * linearly.
     * @details The molecule is spanned on x axis and centered in its mass centre. Primary axis is naturally x axis
     * (positive, towards the largest sphere). Secondary axis is y axis - formally it is degenerate in yz plane, but was
     * arbitrarily chosen to enable flip moves. Geometric centre lies in the centre of a bounding box (it coincides with
     * the mass centre only if all spheres have the same radius). The class specifies custom named points "ss" and "sl"
     * for first (small) and last (large) spheres, together with once inherited from PolysphereTraits.
     * @sa ::PolysphereWedgeTraits
     */
    class PolysphereWedgeTraits : public PolysphereTraits {
    private:
        static PolysphereShape generateShape(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                                             double spherePenetration);

    public:
        /**
         * @brief Constructs the class for hard interactions.
         * @param sphereNum number of all spheres
         * @param smallSphereRadius radius of the smallest (first) sphere
         * @param largeSphereRadius radius of the largest (last) sphere
         * @param spherePenetration how much spheres overlap (in particular 0 means tangent spheres)
         */
        PolysphereWedgeTraits(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                              double spherePenetration)
                : PolysphereTraits(generateShape(sphereNum, smallSphereRadius, largeSphereRadius, spherePenetration))
        { }

        /**
         * @brief Similar as PolysphereWedgeTraits::PolysphereWedgeTraits(std::size_t, double, double, double), but
         * with soft central interaction given by @a centralInteraction.
         */
        PolysphereWedgeTraits(std::size_t sphereNum, double smallSphereRadius, double largeSphereRadius,
                              double spherePenetration, const std::shared_ptr<CentralInteraction> &centralInteraction)
                : PolysphereTraits(generateShape(sphereNum, smallSphereRadius, largeSphereRadius, spherePenetration),
                                   centralInteraction)
        { }
    };
}


/**
 * @brief A class representing linear sphere polymer where radii of the spheres grow linearly.
 * @details <p> The sha[e is spanned on the Z axis and centered in the midpoint of the bounding box in order to minimize
 * the circumsphere radius. The primary axis is naturally the positive Z axis. The class specifies custom named points
 * "beg" and "end" for, respectively, bottom and top spheres, together with the ones inherited from PolysphereTraits.
 * Named point "cm" denoting the mass center is defined only if the spheres do not overlap.
 *
 * <p> The class is a thin decorator of PolysphereTraits, which means that arbitrary PolysphereShape species can be
 * added using addSpecies() method.
 * @sa legacy::PolysphereWedgeTraits
 */
class PolysphereWedgeTraits : public PolysphereTraits {
private:
    static double calculateVolume(const std::vector<SphereData> &sphereData, double spherePenetration);

public:
    /**
     * @brief Generates the PolysphereShape of a wedge, compatible with addSpecies().
     * @param sphereNum number of all spheres. It must be &ge; 2
     * @param bottomSphereRadius radius of the top sphere. It must be positive
     * @param topSphereRadius radius of the bottom sphere. It must be positive
     * @param spherePenetration how much spheres overlap (in particular, 0 yields tangent spheres). It must lie in the
     * range 0 &le; spherePenetration < 2 &middot; min(@a bottomSphereRadius, @a topSphereRadius)
     * @throws PreconditionException if the shape is malformed (see the constraints of the arguments)
     */
    static PolysphereShape generateShape(std::size_t sphereNum, double bottomSphereRadius, double topSphereRadius,
                                         double spherePenetration);
    /**
     * @brief Creates the class with hard-core interactions and no initially registered species.
     */
    PolysphereWedgeTraits() = default;

    /**
     * @brief Creates the class with hard-core interactions and one species named `A` based on the given arguments,
     * which is set as a default species (setDefaultSpecies()). The arguments have the identical meaning as in
     * generateShape().
     */
    PolysphereWedgeTraits(std::size_t sphereNum, double bottomSphereRadius, double topSphereRadius,
                          double spherePenetration)
            : PolysphereTraits(generateShape(sphereNum, bottomSphereRadius, topSphereRadius, spherePenetration))
    { }

    /**
     * @brief Creates the class with soft interactions @a centralInteraction and no initially registered species.
     */
    explicit PolysphereWedgeTraits(const std::shared_ptr<CentralInteraction> &centralInteraction)
            : PolysphereTraits(centralInteraction)
    { }

    /**
     * @brief Creates the class with soft interactions @a centralInteraction and one species named `A` based on the
     * given arguments, which is set as a default species (setDefaultSpecies()). The arguments have the identical
     * meaning as in generateShape().
     */
    PolysphereWedgeTraits(std::size_t sphereNum, double bottomSphereRadius, double topSphereRadius,
                          double spherePenetration, const std::shared_ptr<CentralInteraction> &centralInteraction)
            : PolysphereTraits(generateShape(sphereNum, bottomSphereRadius, topSphereRadius, spherePenetration),
                               centralInteraction)
    { }

    /**
     * @brief Registers a new species named @a shapeName of a wedge given by the rest of the arguments, whose meaning is
     * the same as in generateShape().
     */
    void addLollipopShape(const std::string &shapeName, std::size_t sphereNum, double bottomSphereRadius,
                          double topSphereRadius, double spherePenetration)
    {
        auto shape = PolysphereWedgeTraits::generateShape(sphereNum, bottomSphereRadius, topSphereRadius,
                                                          spherePenetration);
        this->addSpecies(shapeName, shape);
    }
};


#endif //RAMPACK_POLYSPHEREWEDGETRAITS_H
