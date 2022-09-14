//
// Created by pkua on 12.09.22.
//

#include <algorithm>
#include <numeric>
#include <iterator>

#include "RadialEnumerator.h"


void RadialEnumerator::enumeratePairs(const Packing &packing, const ShapeTraits &shapeTraits,
                                      PairConsumer &pairConsumer) const
{
    const auto &bc = packing.getBoundaryConditions();
    auto focalPoints = packing.dumpNamedPoints(shapeTraits.getGeometry(), this->focalPoint);
    for (std::size_t i{}; i < packing.size(); i++) {
        for (std::size_t j = i; j < packing.size(); j++) {
            const Vector<3> &pos1 = focalPoints[i];
            Vector<3> pos2 = focalPoints[j];
            pos2 += bc.getTranslation(pos1, pos2);

            double distance2 = (pos2 - pos1).norm2();
            pairConsumer.consumePair(packing, {i, j}, std::sqrt(distance2));
        }
    }
}

std::vector<double> RadialEnumerator::getExpectedNumOfMoleculesInShells(const Packing &packing,
                                                                        const std::vector<double> &radiiBounds) const
{
    Expects(radiiBounds.size() >= 2);
    Expects(std::is_sorted(radiiBounds.begin(), radiiBounds.end()));


    std::vector<double> molecules;
    molecules.reserve(radiiBounds.size());
    double numberDensity = packing.getNumberDensity();
    auto moleculesInDisk = [numberDensity](double r) { return 4./3 * M_PI * r*r*r * numberDensity; };
    std::transform(radiiBounds.begin(), radiiBounds.end(), std::back_inserter(molecules), moleculesInDisk);

    std::vector<double> result;
    result.reserve(radiiBounds.size() - 1);
    std::transform(std::next(molecules.begin()), molecules.end(), molecules.begin(), std::back_inserter(result),
                   std::minus<>{});
    return result;
}
