//
// Created by pkua on 20.10.22.
//

#ifndef RAMPACK_FOURIERTRACKER_H
#define RAMPACK_FOURIERTRACKER_H

#include "core/observables/GoldestoneTracker.h"


class FourierTracker : GoldestoneTracker {
public:
    [[nodiscard]] std::string getModeName() const override;

    void calculateOrigin(const Packing &packing, const ShapeTraits &shapeTraits) override;
};


#endif //RAMPACK_FOURIERTRACKER_H
