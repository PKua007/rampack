//
// Created by Piotr Kubala on 31/08/2023.
//

#include "LinearEnumerator.h"


std::string LinearEnumerator::getSignatureName() const {
    switch (this->axis) {
        case Axis::X:
            return "x";
        case Axis::Y:
            return "y";
        case Axis::Z:
            return "z";
    }
    AssertThrow("unreachable");
}

std::vector<double> LinearEnumerator::getExpectedNumOfMoleculesInShells(const Packing &packing,
                                                                        const std::vector<double> &radiiBounds) const
{
    Expects(radiiBounds.size() >= 2);
    Expects(std::is_sorted(radiiBounds.begin(), radiiBounds.end()));

    const auto &box = packing.getBox();
    double volume = box.getVolume();
    double height = box.getHeights()[static_cast<std::size_t>(this->axis)];
    double area = volume/height;
    double numberDensity = packing.getNumberDensity();

    std::vector<double> result;
    result.reserve(radiiBounds.size() - 1);
    for (std::size_t i = 0; i < radiiBounds.size() - 1; i++)
        result.push_back((radiiBounds[i + 1] - radiiBounds[i]) * area * numberDensity);

    return result;
}

void LinearEnumerator::enumeratePairs(const Packing &packing, const ShapeTraits &shapeTraits,
                                      PairConsumer &pairConsumer) const
{
    const auto &bc = packing.getBoundaryConditions();
    auto focalPoints = packing.dumpNamedPoints(shapeTraits.getGeometry(), this->focalPointName);
    [[maybe_unused]] std::size_t maxThreads = pairConsumer.getMaxThreads();     // maybe-unused if OpenMP not available

    const auto &sides = packing.getBox().getSides();
    Vector<3> direction;
    switch (this->axis) {
        case Axis::X:
            direction = (sides[1] ^ sides[2]).normalized();
            break;
        case Axis::Y:
            direction = (sides[2] ^ sides[0]).normalized();
            break;
        case Axis::Z:
            direction = (sides[0] ^ sides[1]).normalized();
            break;
    }

    #pragma omp parallel for shared(packing, focalPoints, bc, pairConsumer, shapeTraits, direction) default(none) \
            schedule(dynamic) num_threads(maxThreads)
    for (std::size_t i = 0; i < packing.size(); i++) {
        for (std::size_t j = i; j < packing.size(); j++) {
            const Vector<3> &pos1 = focalPoints[i];
            Vector<3> pos2 = focalPoints[j];
            pos2 += bc.getTranslation(pos1, pos2);

            Vector<3> diff = pos2 - pos1;
            diff = (diff * direction) * direction;
            pairConsumer.consumePair(packing, {i, j}, diff.norm(), diff, shapeTraits);
        }
    }
}
