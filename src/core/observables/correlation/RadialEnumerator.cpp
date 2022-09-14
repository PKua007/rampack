//
// Created by pkua on 12.09.22.
//

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
            double jacobian = 4 * M_PI * distance2 * packing.getNumberDensity();
            pairConsumer.consumePair(packing, {i, j}, std::sqrt(distance2), jacobian);
        }
    }
}
