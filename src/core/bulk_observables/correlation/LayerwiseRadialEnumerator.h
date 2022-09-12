//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_LAYERWISERADIALENUMERATOR_H
#define RAMPACK_LAYERWISERADIALENUMERATOR_H

#include "PairEnumerator.h"


class LayerwiseRadialEnumerator : public PairEnumerator {
public:
    void enumeratePairs(const Packing &packing, PairConsumer &pairConsumer) const override;
};


#endif //RAMPACK_LAYERWISERADIALENUMERATOR_H
