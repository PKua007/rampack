//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_RADIALENUMERATOR_H
#define RAMPACK_RADIALENUMERATOR_H

#include "PairEnumerator.h"


class RadialEnumerator : public PairEnumerator {
private:
    std::string focalPoint;

public:
    explicit RadialEnumerator(std::string focalPoint = "cm") : focalPoint{std::move(focalPoint)} { }

    void enumeratePairs(const Packing &packing, const ShapeTraits &shapeTraits,
                        PairConsumer &pairConsumer) const override;
};


#endif //RAMPACK_RADIALENUMERATOR_H
