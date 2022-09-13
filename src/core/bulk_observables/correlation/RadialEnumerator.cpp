//
// Created by pkua on 12.09.22.
//

#include "RadialEnumerator.h"


void RadialEnumerator::enumeratePairs(const Packing &packing, PairConsumer &pairConsumer) const {
    const auto &bc = packing.getBoundaryConditions();
    for (std::size_t i{}; i < packing.size(); i++) {
        for (std::size_t j = i; j < packing.size(); j++) {
            const Vector<3> &pos1 = packing[i].getPosition();
            Vector<3> pos2 = packing[j].getPosition();
            pos2 += bc.getTranslation(pos1, pos2);

            double distance2 = (pos2 - pos1).norm2();
            double jacobian = 4 * M_PI * distance2 * packing.getNumberDensity();
            pairConsumer.consumePair(packing, {i, j}, std::sqrt(distance2), jacobian);
        }
    }
}
