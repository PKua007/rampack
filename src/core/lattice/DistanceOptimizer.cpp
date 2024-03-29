//
// Created by pkup on 18.06.2021.
//

#include "DistanceOptimizer.h"

#include "utils/Exceptions.h"
#include "LatticeTraits.h"
#include "core/FreeBoundaryConditions.h"
#include "LatticeTransformer.h"


double DistanceOptimizer::minimizeForDirection(Shape shape1, Shape shape2, Vector<3> direction,
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

std::array<double, 3> DistanceOptimizer::minimizeForAxes(const Shape &shape1, const Shape &shape2,
                                                         const Interaction &interaction)
{
    return {DistanceOptimizer::minimizeForDirection(shape1, shape2, {1, 0, 0}, interaction),
            DistanceOptimizer::minimizeForDirection(shape1, shape2, {0, 1, 0}, interaction),
            DistanceOptimizer::minimizeForDirection(shape1, shape2, {0, 0, 1}, interaction)};
}

void DistanceOptimizer::shrinkPacking(Packing &packing, const Interaction &interaction,
                                      const std::string &axisOrderString)
{
    TransformerValidateMsg(interaction.hasHardPart(),
                           "Interaction must have a hard component to perform distance optimization");

    const double range = interaction.getTotalRangeRadius();
    const auto &initialHeights = packing.getBox().getHeights();
    constexpr double FACTOR_EPSILON = 1 + 1e-12;

    // Verify initial dimensions whether they are large enough.
    // Use range (plus epsilon), since we don't want self intersections through PBC
    TransformerValidateMsg(std::all_of(initialHeights.begin(), initialHeights.end(),
                                       [range](double d) { return d > FACTOR_EPSILON*range; }),
                           "Initial dimensions are not large enough to perform distance optimization; particles may "
                           "self-overlap through PBC reflections");

    TransformerValidateMsg(!packing.countTotalOverlaps(interaction),
                           "Overlaps are present at the start of distance optimization");

    std::vector<Shape> relShapes(std::begin(packing), std::end(packing));
    const auto &originalBox = packing.getBox();
    for (auto &shape : relShapes)
        shape.setPosition(originalBox.absoluteToRelative(shape.getPosition()));

    // Optimize axis by axis in a given axis order
    auto axisOrder = LatticeTraits::parseAxisOrder(axisOrderString);
    for (std::size_t axisNum : axisOrder) {
        // Bisection interval beginning and end
        double begFactor = range * FACTOR_EPSILON / initialHeights[axisNum];  // Smallest scaling without self-overlap
        double endFactor = 1;
        auto endBox = packing.getBox().getDimensions();
        auto begBox = packing.getBox().getDimensions();
        for (std::size_t i{}; i < 3; i++)
            begBox(i, axisNum) *= begFactor;
        auto endShapes = DistanceOptimizer::generateAbsoluteShapes(relShapes, endBox);
        auto begShapes = DistanceOptimizer::generateAbsoluteShapes(relShapes, begBox);

        packing.reset(begShapes, TriclinicBox(begBox), interaction);

        TransformerValidateMsg(packing.countTotalOverlaps(interaction),
                               "Maximally shrunk packing (avoiding self overlaps) is not overlapping - cell number on "
                               "axis " + std::string{static_cast<char>('x' + axisNum)} + " is too small");

        do {
            double factorMid = (begFactor + endFactor) / 2;
            auto midBox = (begBox + endBox) / 2.;
            auto midShapes = DistanceOptimizer::generateAbsoluteShapes(relShapes, midBox);
            packing.reset(midShapes, TriclinicBox(midBox), interaction);
            if (packing.countTotalOverlaps(interaction)) {
                begFactor = factorMid;
                begBox = midBox;
            } else {
                endFactor = factorMid;
                endShapes = std::move(midShapes);
                endBox = midBox;
            }
        } while (std::abs(endFactor - begFactor) > EPSILON);

        // Expand maximally shrunk axis a bit for greater numerical stability in the box optimization procedure.
        // In essence, make sure that the error margin is always between FACTOR_EPSILON and 2*FACTOR_EPSILON
        // (which would be 0 to FACTOR_EPSILON without this step - bisection may find the tangent position arbitrarily
        // close "by accident")
        for (std::size_t i{}; i < 3; i++)
            endBox(i, axisNum) *= FACTOR_EPSILON;

        // Finally, apply the factor found
        packing.reset(endShapes, TriclinicBox(endBox), interaction);
        Assert(!packing.countTotalOverlaps(interaction));
    }
}

std::vector<Shape> DistanceOptimizer::generateAbsoluteShapes(std::vector<Shape> relShapes,
                                                             const Matrix<3, 3> &boxMatrix)
{
    for (auto &shape : relShapes)
        shape.setPosition(boxMatrix * shape.getPosition());
    return relShapes;
}
