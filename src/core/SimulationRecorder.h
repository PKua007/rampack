//
// Created by pkua on 04.04.2022.
//

#ifndef RAMPACK_SIMULATIONRECORDER_H
#define RAMPACK_SIMULATIONRECORDER_H

#include <iostream>
#include <memory>

#include "Packing.h"


class SimulationRecorder {
private:
    struct Header {
        char magic[6] = {'R', 'A', 'M', 'T', 'R', 'J'};
        char newline = '\n';
        unsigned char versionMajor = 1;
        unsigned char versionMinor = 0;
        std::size_t numParticles{};
        std::size_t numSnapshots{};
        std::size_t cycleStep{};
    };

    struct BoxData {
        double dimensions[9]{};
    };

    struct ParticleData {
        double position[3]{};
        double eulerAngles[3]{};
    };

    std::unique_ptr<std::iostream> stream;
    std::size_t numSnapshots{};
    std::size_t cycleStep{};
    std::size_t numParticles{};

    static Header readHeader(std::istream &stream);

public:
    SimulationRecorder(std::unique_ptr<std::iostream> stream, bool append);
    ~SimulationRecorder();

    void recordSnapshot(const Packing &packing, std::size_t cycle);
    void close();
};


#endif //RAMPACK_SIMULATIONRECORDER_H
