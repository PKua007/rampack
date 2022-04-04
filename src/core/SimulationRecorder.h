//
// Created by pkua on 04.04.2022.
//

#ifndef RAMPACK_SIMULATIONRECORDER_H
#define RAMPACK_SIMULATIONRECORDER_H

#include <iostream>
#include <memory>

#include "Packing.h"
#include "SimulationIO.h"


class SimulationRecorder : SimulationIO {
private:
    std::unique_ptr<std::iostream> stream;
    std::size_t numSnapshots{};
    std::size_t cycleStep{};
    std::size_t numParticles{};

public:
    SimulationRecorder(std::unique_ptr<std::iostream> stream, bool append);
    ~SimulationRecorder();

    void recordSnapshot(const Packing &packing, std::size_t cycle);
    void close();
};


#endif //RAMPACK_SIMULATIONRECORDER_H
