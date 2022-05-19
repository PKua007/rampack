//
// Created by pkup on 18.06.2021.
//

#include "DistanceOptimizer.h"

#include "utils/Assertions.h"
#include "utils/Utils.h"
#include "FreeBoundaryConditions.h"

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
    Expects(interaction.hasHardPart());

    const double range = interaction.getTotalRangeRadius();
    const auto &initialDim = packing.getBox().getHeights();
    constexpr double FACTOR_EPSILON = 1 + 1e-12;

    // Verify initial dimensions whether they are large enough
    // range (plus epsilon), since we don't want self intersections through PBC
    Expects(std::all_of(initialDim.begin(), initialDim.end(), [range](double d) { return d > FACTOR_EPSILON*range; }));
    Expects(!isPackingOverlapping(packing, interaction));

    // Optimize axis by axis in a given axis order
    auto axisOrder = parse_axis_order(axisOrderString);
    for (std::size_t axisNum : axisOrder) {
        double factorBeg = range * FACTOR_EPSILON / initialDim[axisNum];  // Smallest scaling without self-overlap
        double factorEnd = 1;

        std::array<double, 3> testFactors = singleAxisScaling(axisNum, factorBeg);
        ExpectsMsg(isScaledPackingOverlapping(packing, interaction, testFactors),
                   "Maximally shrunk packing (avoiding self overlaps) is not overlapping - to little particles");

        do {
            double factorMid = (factorBeg + factorEnd) / 2;
            if (isScaledPackingOverlapping(packing, interaction, singleAxisScaling(axisNum, factorMid)))
                factorBeg = factorMid;
            else
                factorEnd = factorMid;
        } while (std::abs(factorEnd - factorBeg) > EPSILON);

        // Finally, apply the factor found
        double finalEnergy = packing.tryScaling(singleAxisScaling(axisNum, factorEnd), interaction);
        Assert(finalEnergy != INF);
    }
}

bool DistanceOptimizer::isPackingOverlapping(Packing &packing, const Interaction &interaction) {
    return packing.tryScaling(1, interaction) == INF;
}

std::array<double, 3> DistanceOptimizer::singleAxisScaling(std::size_t axisNum, double factor) {
    std::array<double, 3> testFactors{};
    testFactors.fill(1);
    testFactors[axisNum] = factor;
    return testFactors;
}

bool DistanceOptimizer::isScaledPackingOverlapping(Packing &packing, const Interaction &interaction,
                                                   const std::array<double, 3> &factors)
{
    bool isOverlapping = (packing.tryScaling(factors, interaction) == INF);
    packing.revertScaling();
    return isOverlapping;
}
