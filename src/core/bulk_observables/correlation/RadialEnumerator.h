//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_RADIALENUMERATOR_H
#define RAMPACK_RADIALENUMERATOR_H

#include "PairEnumerator.h"


class RadialEnumerator : public PairEnumerator {
public:
    void enumeratePairs(const Packing &packing, PairConsumer &pairConsumer) const override;
};


#endif //RAMPACK_RADIALENUMERATOR_H
