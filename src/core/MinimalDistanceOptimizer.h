//
// Created by pkup on 18.06.2021.
//

#ifndef RAMPACK_MINIMALDISTANCEOPTIMIZER_H
#define RAMPACK_MINIMALDISTANCEOPTIMIZER_H

#include <array>

#include "Shape.h"
#include "Packing.h"
#include "Interaction.h"
#include "geometry/Vector.h"

class MinimalDistanceOptimizer {
private:
    static std::array<std::size_t, 3> parseAxisOrder(const std::string &axisOrderString);

public:
    static constexpr double EPSILON = 1e-12;

    static double forDirection(Shape shape1, Shape shape2, Vector<3> direction, const Interaction &interaction);
    static std::array<double, 3> forAxes(const Shape &shape1, const Shape &shape2, const Interaction &interaction);
    static void shrinkPacking(Packing &packing, const Interaction &interaction, const std::string &axisOrderString);
};


#endif //RAMPACK_MINIMALDISTANCEOPTIMIZER_H
