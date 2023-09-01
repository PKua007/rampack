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
    auto focalPoints = packing.dumpNamedPoints(shapeTraits.getGeometry(), this->focalPointName);
    [[maybe_unused]] std::size_t maxThreads = pairConsumer.getMaxThreads();     // maybe-unused if OpenMP not available

    #pragma omp parallel for shared(packing, focalPoints, bc, pairConsumer, shapeTraits) default(none) \
            schedule(dynamic) num_threads(maxThreads)
    for (std::size_t i = 0; i < packing.size(); i++) {
        for (std::size_t j = i; j < packing.size(); j++) {
            const Vector<3> &pos1 = focalPoints[i];
            Vector<3> pos2 = focalPoints[j];
            pos2 += bc.getTranslation(pos1, pos2);

            Vector<3> diff = pos2 - pos1;
            pairConsumer.consumePair(packing, {i, j}, diff.norm(), diff, shapeTraits);
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
    auto moleculesInBall = [numberDensity](double r) { return 4. / 3 * M_PI * r * r * r * numberDensity; };
    std::transform(radiiBounds.begin(), radiiBounds.end(), std::back_inserter(molecules), moleculesInBall);

    std::vector<double> result;
    result.reserve(radiiBounds.size() - 1);
    std::transform(std::next(molecules.begin()), molecules.end(), molecules.begin(), std::back_inserter(result),
                   std::minus<>{});
    return result;
}
