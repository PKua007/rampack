//
// Created by pkua on 12.09.22.
//

#ifndef RAMPACK_LAYERWISERADIALENUMERATOR_H
#define RAMPACK_LAYERWISERADIALENUMERATOR_H

#include <array>

#include "PairEnumerator.h"


class LayerwiseRadialEnumerator : public PairEnumerator {
private:
    Vector<3> millerIndices{};
    std::string focalPoint{};

public:
    explicit LayerwiseRadialEnumerator(const std::array<std::size_t, 3> &millerIndices, std::string focalPoint = "cm");

    void enumeratePairs(const Packing &packing, const ShapeTraits &shapeTraits,
                        PairConsumer &pairConsumer) const override;
};


#endif //RAMPACK_LAYERWISERADIALENUMERATOR_H
