//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_POLYSPHEREBANANATRAITS_H
#define RAMPACK_POLYSPHEREBANANATRAITS_H

#include "GenericPolysphereTraits.h"


namespace legacy {
    /**
     * @brief The legacy version (pre 0.2.0) of the banana-shaped hard polymer built by placing spheres on an arc. The
     * sphere can be tangent or overlapping, depending on the parameters.
     * @details The arc lies in the xy plane with x = 0, y = 0 origin, symmetric w.r.t. x axis and lying in negative
     * x half-plane. The primary (molecular) axis is thus y axis, while secondary (polarization) axis is negative x
     * axis. Geometric origin is placed in the mass centre. The class specifies custom named points "sbeg" and "send"
     * for first and last spheres, together with the ones inherited from PolysphereTraits.
     * @sa ::PolysphereBananaTraits
     */
    class PolysphereBananaTraits : public GenericPolysphereTraits {
    private:
        static PolysphereShape generateShape(double arcRadius, double arcAngle, std::size_t sphereNum,
                                             double sphereRadius);

    public:
        /**
         * @brief Constructs the shape.
         * @param arcRadius the radius of the arc
         * @param arcAngle the arc angle
         * @param sphereNum number of spheres to be equidistantly placed on the arc. First and last sphere centres are arc
         * endpoints
         * @param sphereRadius the radius of each sphere
         */
        PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius)
                : GenericPolysphereTraits(generateShape(arcRadius, arcAngle, sphereNum, sphereRadius))
        { }

        /**
         * @brief Similar as PolysphereBananaTraits::PolysphereBananaTraits(double, double, std::size_t, double), but
         * for soft central interactions given by @a centralInteraction.
         */
        PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius,
                               const std::shared_ptr<CentralInteraction> &centralInteraction)
                : GenericPolysphereTraits(generateShape(arcRadius, arcAngle, sphereNum, sphereRadius), centralInteraction)
        { }
    };
}


/**
 * @brief The banana-shaped hard polymer built by placing spheres on an arc. The spheres can be tangent or overlapping,
 * depending on the parameters.
 * @details <p> The arc lies in the XZ plane, symmetric w.r.t. the X axis and it is bent towards the negative X
 * half-space. For @a arcRadius smaller than \f$\pi\f$, the origin lies in the middle of the line joining the endpoints,
 * while for larger @a arcRadius it coincides with the arc's midpoint (it is defined in such a way to minimize the
 * circumsphere radius). The primary (molecular) axis is the Z axis, while the secondary axis is the negative X axis.
 * The class specifies custom named points "beg" and "end" for the endpoint spheres, respectively with a negative and
 * a positive z coordinate, together with the ones inherited from PolysphereTraits. Named point "cm" denoting the mass
 * center is defined only if the spheres do not overlap.
 *
 * <p> The class is a thin decorator of PolysphereTraits, which means that arbitrary PolysphereShape species can be
 * added using addSpecies() method.
 * @sa legacy::PolysphereBananaTraits
 */
class PolysphereBananaTraits : public GenericPolysphereTraits {
private:
    static double calculateVolume(const std::vector<SphereData> &sphereData, double arcAngle);
    static void addMassCentre(PolysphereShape &shape);

public:
    /**
     * @brief Generates the PolysphereShape of a banana, compatible with addSpecies().
     * @param arcRadius the radius of the arc. It must lie in the range 0 < @a arcRadius &le; @a sphereRadius
     * @param arcAngle the arc angle. It must lie in the range 0 < @a arcAngle < 2&pi;
     * @param sphereNum number of spheres to be equidistantly placed on the arc. First and last sphere centers are the
     * arc endpoints. It must be &ge; 2
     * @param sphereRadius the radius of each sphere
     * @throws PreconditionException if the shape is malformed (see the constraint of the arguments)
     */
    static PolysphereShape generateShape(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius);

    /**
     * @brief Creates the class with hard-core interactions and no initially registered species.
     */
    PolysphereBananaTraits() = default;

    /**
     * @brief Creates the class with hard-core interactions and one species named `A` based on the given arguments,
     * which is set as a default species (setDefaultSpecies()). The arguments have the identical meaning as in
     * generateShape().
     */
    PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius)
            : GenericPolysphereTraits(generateShape(arcRadius, arcAngle, sphereNum, sphereRadius))
    { }

    /**
     * @brief Creates the class with soft interactions @a centralInteraction and no initially registered species.
     */
    explicit PolysphereBananaTraits(const std::shared_ptr<CentralInteraction> &centralInteraction)
            : GenericPolysphereTraits(centralInteraction)
    { }

    /**
     * @brief Creates the class with soft interactions @a centralInteraction and one species named `A` based on the
     * given arguments, which is set as a default species (setDefaultSpecies()). The arguments have the identical
     * meaning as in generateShape().
     */
    PolysphereBananaTraits(double arcRadius, double arcAngle, std::size_t sphereNum, double sphereRadius,
                           const std::shared_ptr<CentralInteraction> &centralInteraction)
            : GenericPolysphereTraits(generateShape(arcRadius, arcAngle, sphereNum, sphereRadius), centralInteraction)
    { }

    /**
     * @brief Registers a new species named @a shapeName of a banana given by the rest of the arguments, whose meaning
     * is the same as in generateShape().
     */
    void addBananaShape(const std::string &shapeName, double arcRadius, double arcAngle, std::size_t sphereNum,
                        double sphereRadius)
    {
        this->addSpecies(shapeName,
                         PolysphereBananaTraits::generateShape(arcRadius, arcAngle, sphereNum, sphereRadius));
    }
};


#endif //RAMPACK_POLYSPHEREBANANATRAITS_H
