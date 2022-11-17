//
// Created by pkua on 17.11.22.
//

#ifndef RAMPACK_SIMULATIONRECORDER_H
#define RAMPACK_SIMULATIONRECORDER_H

#include "Packing.h"


class SimulationRecorder {
public:
    virtual ~SimulationRecorder() = default;
    virtual void recordSnapshot(const Packing &packing, std::size_t cycle) = 0;
    virtual void close() = 0;
};


#endif //RAMPACK_SIMULATIONRECORDER_H
