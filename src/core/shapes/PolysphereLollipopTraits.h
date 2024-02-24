//
// Created by pkua on 01.03.2022.
//

#ifndef RAMPACK_POLYSPHERELOLLIPOPTRAITS_H
#define RAMPACK_POLYSPHERELOLLIPOPTRAITS_H

#include "PolysphereTraits.h"


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
    class PolysphereLollipopTraits : public PolysphereTraits {
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
                : PolysphereTraits(generateShape(sphereNum, smallSphereRadius, largeSphereRadius,
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
                : PolysphereTraits(generateShape(sphereNum, smallSphereRadius, largeSphereRadius,
                                                 smallSpherePenetration, largeSpherePenetration),
                                   centralInteraction)
        { }
    };
}


/**
 * @brief A class representing linear sphere polymer capped with one sphere of a different size(lollipop-shaped).
 * @details The molecule is spanned on z axis and centered in the centre of a bounding box to minimize the circumsphere
 * radius. Primary axis is naturally z axis (positive, towards the large sphere). Secondary axis is x axis - formally it
 * is degenerate in xz plane, but was arbitrarily chosen to enable flip moves. The class specifies custom named points
 * "ss" and "st" for the first (the end of lollipop's stick) and the last (the tip of the lollipop) sphere, together
 * with the ones inherited from PolysphereTraits. Mass centre "cm" named point is defined only if both
 * @a smallSpherePenetration and @a largeSpherePenetration are zero.
 * @sa legacy::PolysphereLollipopTraits
 */
class PolysphereLollipopTraits : public PolysphereTraits {
private:
    static double calculateVolume(const std::vector<SphereData> &sphereData, double stickSpherePenetration,
                                  double tipSpherePenetration);

public:
    static PolysphereShape generateShape(std::size_t sphereNum, double stickSphereRadius, double tipSphereRadius,
                                         double stickSpherePenetration, double tipSpherePenetration);

    PolysphereLollipopTraits() = default;

    /**
     * @brief Constructs the class for hard interactions.
     * @param sphereNum number of all spheres - there is @a sphereNum - 1 small spheres and a single large one
     * @param stickSphereRadius radius of sphere building lollipop's stick
     * @param tipSphereRadius radius of a lollipop's tip sphere
     * @param stickSpherePenetration how much stick spheres overlap (in particular 0 means tangent spheres)
     * @param tipSpherePenetration hom much the tip sphere overlaps with the last small
     */
    PolysphereLollipopTraits(std::size_t sphereNum, double stickSphereRadius, double tipSphereRadius,
                             double stickSpherePenetration, double tipSpherePenetration)
            : PolysphereTraits(generateShape(sphereNum, stickSphereRadius, tipSphereRadius,
                                             stickSpherePenetration, tipSpherePenetration))
    { }

    explicit PolysphereLollipopTraits(const std::shared_ptr<CentralInteraction> &centralInteraction)
            : PolysphereTraits(centralInteraction)
    { }

    /**
     * @brief Similar as
     * PolysphereLollipopTraits::PolysphereLollipopTraits(std::size_t, double, double, double, double), but with soft
     * central interaction given by @a centralInteraction.
     */
    PolysphereLollipopTraits(std::size_t sphereNum, double stickSphereRadius, double tipSphereRadius,
                             double stickSpherePenetration, double tipSpherePenetration,
                             const std::shared_ptr<CentralInteraction> &centralInteraction)
            : PolysphereTraits(generateShape(sphereNum, stickSphereRadius, tipSphereRadius,
                                             stickSpherePenetration, tipSpherePenetration),
                               centralInteraction)
    { }

    void addLollipopShape(const std::string &shapeName, std::size_t sphereNum, double stickSphereRadius,
                          double tipSphereRadius, double stickSpherePenetration, double tipSpherePenetration)
    {
        auto shape = PolysphereLollipopTraits::generateShape(sphereNum, stickSphereRadius, tipSphereRadius,
                                                             stickSpherePenetration, tipSpherePenetration);
        this->addPolysphereShape(shapeName, shape);
    }
};



#endif //RAMPACK_POLYSPHERELOLLIPOPTRAITS_H
