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

void MinimalDistanceOptimizer::shrinkPacking(Packing &packing, const Interaction &interaction,
                                             const std::string &axisOrderString)
{
    Expects(interaction.hasHardPart());

    const double range = interaction.getTotalRangeRadius();
    const auto &dim = packing.getDimensions();
    constexpr double FACTOR_EPSILON = 1 + 1e-12;

    // Verify initial dimensions whether they are large enough
    // range (plus epsilon), since we don't want self intersections through PBC
    Expects(std::all_of(dim.begin(), dim.end(), [range](double d) { return d > FACTOR_EPSILON*range; }));

    // Verify that initially packing has no intersections
    constexpr double INF = std::numeric_limits<double>::infinity();
    Expects(packing.tryScaling(1, interaction) != INF);

    auto axisOrder = parseAxisOrder(axisOrderString);
    for (std::size_t axisNum : axisOrder) {
        double factorBeg = range*FACTOR_EPSILON/dim[axisNum];
        double factorEnd = 1;

        // Verify that left bisection endpoint (scaling factor) give intersecting packing
        std::array<double, 3> testFactors{};
        testFactors.fill(1);
        testFactors[axisNum] = factorBeg;
        Expects(packing.tryScaling(testFactors, interaction) == INF);
        packing.revertScaling();

        do {
            double factorMid = (factorBeg + factorEnd) / 2;
            std::array<double, 3> factors{};
            factors.fill(1);
            factors[axisNum] = factorMid;
            if (packing.tryScaling(factors, interaction) == INF)
                factorBeg = factorMid;
            else
                factorEnd = factorMid;
            packing.revertScaling();
        } while (std::abs(factorEnd - factorBeg) > EPSILON);

        // Finally, apply the factor found
        std::array<double, 3> factors{};
        factors.fill(1);
        factors[axisNum] = factorEnd;
        double energy = packing.tryScaling(factors, interaction);
        Assert(energy != INF);
    }
}

std::array<std::size_t, 3> MinimalDistanceOptimizer::parseAxisOrder(const std::string &axisOrderString) {
    if (axisOrderString == "xyz")
        return {0, 1, 2};
    else if (axisOrderString == "xzy")
        return {0, 2, 1};
    else if (axisOrderString == "yxz")
        return {1, 0, 2};
    else if (axisOrderString == "yzx")
        return {1, 2, 0};
    else if (axisOrderString == "zxy")
        return {2, 0, 1};
    else if (axisOrderString == "zyx")
        return {2, 1, 0};
    else
        throw PreconditionException("Malformed axis order");
}
