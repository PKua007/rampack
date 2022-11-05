//
// Created by pkua on 31.10.22.
//

#ifndef RAMPACK_DUMMYTRACKER_H
#define RAMPACK_DUMMYTRACKER_H

#include "core/observables/GoldstoneTracker.h"


/**
 * @brief Dummy tracker which does not track anything - it leaves origin position and system orientation at default
 * values.
 */
class DummyTracker : public GoldstoneTracker {
public:
    [[nodiscard]] std::string getTrackingMethodName() const override { return "dummy"; }
    void calculateOrigin([[maybe_unused]] const Packing &packing,
                         [[maybe_unused]] const ShapeTraits &shapeTraits) override
    { }
    void reset() override { }
};


#endif //RAMPACK_DUMMYTRACKER_H
