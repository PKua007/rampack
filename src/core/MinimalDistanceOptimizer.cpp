//
// Created by pkup on 18.06.2021.
//

#include "MinimalDistanceOptimizer.h"

#include "utils/Assertions.h"
#include "FreeBoundaryConditions.h"

double MinimalDistanceOptimizer::forDirection(Shape shape1, Shape shape2, Vector<3> direction,
                                              const Interaction &interaction)
{
    Expects(direction.norm2() > EPSILON);
    Expects(interaction.hasHardPart());

    FreeBoundaryConditions fbc;
    direction = direction.normalized();
    shape1.setPosition(Vector<3>{});
    shape2.setPosition(Vector<3>{});
    Assert(interaction.overlapBetweenShapes(shape1, shape2, fbc));

    // Exponentially find an upper bound for distance without overlap
    double distanceMax{1};
    shape2.setPosition(distanceMax * direction);
    while (interaction.overlapBetweenShapes(shape1, shape2, fbc)) {
        distanceMax *= 2;
        shape2.setPosition(distanceMax * direction);
        Assert(distanceMax < 1000);
    }

    // Bisectively find precise tangent distance
    double distanceMin{0};
    do {
        double distanceMid = (distanceMax + distanceMin) / 2;
        shape2.setPosition(distanceMid * direction);
        if (interaction.overlapBetweenShapes(shape1, shape2, fbc))
            distanceMin = distanceMid;
        else
            distanceMax = distanceMid;
    } while(distanceMax - distanceMin > EPSILON);

    // Return optimized distanceMax, which is at most EPSILON from precise tangent distance
    return distanceMax;
}

std::array<double, 3> MinimalDistanceOptimizer::forAxes(const Shape &shape1, const Shape &shape2,
                                                        const Interaction &interaction)
{
    return {MinimalDistanceOptimizer::forDirection(shape1, shape2, {1, 0, 0}, interaction),
            MinimalDistanceOptimizer::forDirection(shape1, shape2, {0, 1, 0}, interaction),
            MinimalDistanceOptimizer::forDirection(shape1, shape2, {0, 0, 1}, interaction)};
}
