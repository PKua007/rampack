//
// Created by pkup on 18.06.2021.
//

#ifndef RAMPACK_DISTANCEOPTIMIZER_H
#define RAMPACK_DISTANCEOPTIMIZER_H

#include <array>
#include <limits>

#include "Shape.h"
#include "Packing.h"
#include "Interaction.h"
#include "geometry/Vector.h"

class DistanceOptimizer {
private:
    static std::array<std::size_t, 3> parseAxisOrder(const std::string &axisOrderString);
    static std::array<double, 3> singleAxisScaling(size_t axisNum, double factor);
    static bool isPackingOverlapping(Packing &packing, const Interaction &interaction);
    static bool isScaledPackingOverlapping(Packing &packing, const Interaction &interaction,
                                           const std::array<double, 3> &factors);

public:
    static constexpr double EPSILON = 1e-12;
    static constexpr double INF = std::numeric_limits<double>::infinity();

    static double minimizeForDirection(Shape shape1, Shape shape2, Vector<3> direction, const Interaction &interaction);
    static std::array<double, 3> minimizeForAxes(const Shape &shape1, const Shape &shape2, const Interaction &interaction);
    static void shrinkPacking(Packing &packing, const Interaction &interaction, const std::string &axisOrderString);
};


#endif //RAMPACK_DISTANCEOPTIMIZER_H
