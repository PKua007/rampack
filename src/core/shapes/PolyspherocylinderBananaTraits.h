//
// Created by Piotr Kubala on 26/04/2021.
//

#ifndef RAMPACK_POLYSPHEROCYLINDERBANANATRAITS_H
#define RAMPACK_POLYSPHEROCYLINDERBANANATRAITS_H

#include "PolyspherocylinderTraits.h"


/**
 * @brief A bent shaped molecule built of hard spherocylinders placed on an arc. The construction is analogous to
 * PolysphereBananaTraits.
 * @details The class specifies custom named points "beg" and "end" for the endpoints of the banana, together with the
 * ones inherited from PolyspherocylinderTraits (see PolyspherocylinderShape::PolyspherocylinderShape). The class is a
 * thin decorator of PolyspherocylinderTraits, which means that arbitrary PolyspherocylinderShape species can be added
 * using addSpecies() method.
 */
class PolyspherocylinderBananaTraits : public PolyspherocylinderTraits {
private:
    static double calculateVolume(double arcRadius, double arcAngle, std::size_t segmentsNum, double radius);

    static void basicValidation(double arcRadius, double arcAngle, std::size_t segmentsNum, double radius);

public:
    /**
     * @brief Generates the PolyspherocylinderShape of a banana, compatible with addSpecies().
     * @param arcRadius the radius of the arc. It must be positive
     * @param arcAngle the arc angle. It must lie in the range 0 < @a arcAngle < 2&pi;
     * @param segmentsNum number of segments to divide the arc into. It must be &ge; 2
     * @param radius radius (half-length) of spherocylinder. It must be positive
     * @param subdivisions a number of sub-parts each spherocylinder should be partitioned into. It does not change the
     * shape, but decreases the interaction range of a single interaction center, which can result in the increase of
     * the simulation throughput. 0 subdivisions is equivalent to 1 and it means that no subdivision is performed
     * @throws PreconditionException if the shape is malformed, i.e. the arguments do not conform to their constraints
     * (as stated in their description), or either of the conditions is not met:
     * - the endpoints caps must not overlap (see isArcOpen())
     * - if @a segmentsNum &ge; 0, the arc origin must lie outside of the shape (see isArcOriginOutside())
     */
    static PolyspherocylinderShape generateShape(double arcRadius, double arcAngle, std::size_t segmentsNum,
                                                 double radius, std::size_t subdivisions = 0);

    /**
     * @brief Returns @a true if the specified arguments (with the same meaning as in generateShape()) yield
     * polyspherocylinder with arc's origin lying outside of the shape, @a false otherwise.
     * @throws PreconditionException if the arguments do not follow their constraints (see generateShape())
     */
    static bool isArcOriginOutside(double arcRadius, double arcAngle, std::size_t segmentsNum, double radius);

    /**
     * @brief Returns @a true if the specified arguments (with the same meaning as in generateShape()) yield
     * polyspherocylinder whose end caps do not overlap, @a false otherwise.
     * @throws PreconditionException if the arguments do not follow their constraints (see generateShape())
     */
    static bool isArcOpen(double arcRadius, double arcAngle, std::size_t segmentsNum, double radius);

    /**
     * @brief Creates the class with no initially registered species.
     */
    PolyspherocylinderBananaTraits() = default;

    /**
     * @brief Creates the class with one species named `A` based on the given arguments,  which is set as a default
     * species (setDefaultSpecies()). The arguments have the identical meaning as in generateShape().
     */
    PolyspherocylinderBananaTraits(double arcRadius, double arcAngle, std::size_t segmentsNum, double radius,
                                   std::size_t subdivisions = 0)
            : PolyspherocylinderTraits(generateShape(arcRadius, arcAngle, segmentsNum, radius, subdivisions))
    { }
};


#endif //RAMPACK_POLYSPHEROCYLINDERBANANATRAITS_H
