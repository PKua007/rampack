//
// Created by pkua on 31.10.22.
//

#ifndef RAMPACK_DUMMYTRACKER_H
#define RAMPACK_DUMMYTRACKER_H

#include "core/observables/GoldestoneTracker.h"


class DummyTracker : public GoldestoneTracker {
public:
    [[nodiscard]] std::string getModeName() const override { return "dummy"; }
    void calculateOrigin([[maybe_unused]] const Packing &packing,
                         [[maybe_unused]] const ShapeTraits &shapeTraits) override
    { }
};


#endif //RAMPACK_DUMMYTRACKER_H
