//
// Created by pkup on 18.06.2021.
//

#ifndef RAMPACK_DISTANCEOPTIMIZER_H
#define RAMPACK_DISTANCEOPTIMIZER_H

#include <array>
#include <limits>

#include "core/Shape.h"
#include "core/Packing.h"
#include "core/Interaction.h"
#include "geometry/Vector.h"

/**
 * @brief An auxiliary class with a couple of static methods enabling one to find the minimal distances between
 * single particles or even whole packings.
 */
class DistanceOptimizer {
private:
    static std::array<double, 3> singleAxisScaling(size_t axisNum, double factor);
    static bool isPackingOverlapping(Packing &packing, const Interaction &interaction);
    static bool isScaledPackingOverlapping(Packing &packing, const Interaction &interaction,
                                           const TriclinicBox &newBox);

    static constexpr double INF = std::numeric_limits<double>::infinity();

public:
    /**
     * @brief The precision to which the minimal distances will be found
     */
    static constexpr double EPSILON = 1e-12;

    /**
     * @brief For shapes @a shape1 and @a shape2 interacting via @a interaction, it finds the minimal distance between
     * them when their centers are displaced on a line spanned by @a direction.
     */
    static double minimizeForDirection(Shape shape1, Shape shape2, Vector<3> direction, const Interaction &interaction);

    /**
     * @brief The function applies DistanceOptimizer::minimizeForDirection to all coordinate system axes: x, y and z
     * and returns an array of minimal distances.
     * @details It can be used for example to estimate the cubic lattice spacing.
     */
    static std::array<double, 3> minimizeForAxes(const Shape &shape1, const Shape &shape2,
                                                 const Interaction &interaction);

    /**
     * @brief The function scales positions of molecules in a given Packing in all directions so that the particles
     * are tangent.
     * @details The scaling is done axis-by-axis in the order given by @a axisOrderString, so it is essentially
     * a single step of "coord descent" optimization scheme.
     * @param packing the packing to shrink
     * @param interaction the interaction between the molecules in the packing
     * @param axisOrderString in which order the axes should be shrunk - for eaxmple @a "xzy" means that x axis will
     * be optimized first, then y and z at the end
     */
    static void shrinkPacking(Packing &packing, const Interaction &interaction, const std::string &axisOrderString);
};


#endif //RAMPACK_DISTANCEOPTIMIZER_H
