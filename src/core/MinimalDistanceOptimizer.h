//
// Created by pkup on 18.06.2021.
//

#ifndef RAMPACK_MINIMALDISTANCEOPTIMIZER_H
#define RAMPACK_MINIMALDISTANCEOPTIMIZER_H

#include <array>

#include "Shape.h"
#include "Interaction.h"
#include "geometry/Vector.h"

class MinimalDistanceOptimizer {
public:
    static constexpr double EPSILON = 1e-12;

    static double forDirection(Shape shape1, Shape shape2, Vector<3> direction, const Interaction &interaction);
    static std::array<double, 3> forAxes(const Shape &shape1, const Shape &shape2, const Interaction &interaction);
};


#endif //RAMPACK_MINIMALDISTANCEOPTIMIZER_H
