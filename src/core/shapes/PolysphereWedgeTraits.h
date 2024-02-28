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
 * @brief A class representing linear sphere polymer where radii of spheres grow linearly.
 * @details The molecule is spanned on z axis and centered in the center of the bounding box in order to minimize the
 * circumsphere radius. Primary axis is naturally positive z axis. Secondary axis is x axis - formally it is degenerate
 * in xz plane, but was arbitrarily chosen to enable flip moves. The class specifies custom named points "beg" and "end"
 * for bottom and top spheres, together with once inherited from PolysphereTraits. Mass centre "cm" named point is
 * defined only if @a spherePenetration is zero.
 * @sa legacy::PolysphereWedgeTraits
 */
class PolysphereWedgeTraits : public PolysphereTraits {
private:
    static double calculateVolume(const std::vector<SphereData> &sphereData, double spherePenetration);

public:
    static PolysphereShape generateShape(std::size_t sphereNum, double bottomSphereRadius, double topSphereRadius,
                                         double spherePenetration);

    PolysphereWedgeTraits() = default;

    /**
     * @brief Constructs the class for hard interactions.
     * @param sphereNum number of all spheres
     * @param topSphereRadius radius of the bottom sphere
     * @param bottomSphereRadius radius of the top sphere
     * @param spherePenetration how much spheres overlap (in particular 0 means tangent spheres)
     */
    PolysphereWedgeTraits(std::size_t sphereNum, double bottomSphereRadius, double topSphereRadius,
                          double spherePenetration)
            : PolysphereTraits(generateShape(sphereNum, bottomSphereRadius, topSphereRadius, spherePenetration))
    { }

    explicit PolysphereWedgeTraits(const std::shared_ptr<CentralInteraction> &centralInteraction)
            : PolysphereTraits(centralInteraction)
    { }

    /**
     * @brief Similar as PolysphereWedgeTraits::PolysphereWedgeTraits(std::size_t, double, double, double), but with
     * soft central interaction given by @a centralInteraction.
     */
    PolysphereWedgeTraits(std::size_t sphereNum, double bottomSphereRadius, double topSphereRadius,
                          double spherePenetration, const std::shared_ptr<CentralInteraction> &centralInteraction)
            : PolysphereTraits(generateShape(sphereNum, bottomSphereRadius, topSphereRadius, spherePenetration),
                               centralInteraction)
    { }

    void addLollipopShape(const std::string &shapeName, std::size_t sphereNum, double bottomSphereRadius,
                          double topSphereRadius, double spherePenetration)
    {
        auto shape = PolysphereWedgeTraits::generateShape(sphereNum, bottomSphereRadius, topSphereRadius,
                                                          spherePenetration);
        this->addShape(shapeName, shape);
    }
};


#endif //RAMPACK_POLYSPHEREWEDGETRAITS_H
