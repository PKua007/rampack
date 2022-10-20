//
// Created by pkua on 04.04.2022.
//

#ifndef RAMPACK_SIMULATIONRECORDER_H
#define RAMPACK_SIMULATIONRECORDER_H

#include <iostream>
#include <memory>

#include "Packing.h"
#include "SimulationIO.h"


/**
 * @brief A class which enables recording simulation to a binary format.
 */
class SimulationRecorder : SimulationIO {
private:
    std::unique_ptr<std::iostream> stream;
    std::size_t numSnapshots{};
    std::size_t cycleStep{};
    std::size_t numParticles{};

public:
    /**
     * @brief Constructs the recorder using a given @a std::iostream.
     * @details The class takes full responsibility of the stream. It should be opened in binary input-output mode with
     * all stream pointer methods working (@a tellp, @a seekp, @a tellg, @a seekg). If @a append is @a true, new
     * snapshots will be appended and it is assumed that the @a stream alredy contains correct recording. If @a append
     * is @a false, the stream should be empty, or else an error is reported.
     */
    SimulationRecorder(std::unique_ptr<std::iostream> stream, std::size_t numParticles, std::size_t cycleStep,
                       bool append);

    ~SimulationRecorder();

    /**
     * @brief Records the next snapshot.
     * @details For each invocation, @a cycle should be a subsequent multiple of cycle step size (which is detected
     * automatically). For example, subsequent invocations can be with @a cycle equal 200, 400, 600, etc.
     */
    void recordSnapshot(const Packing &packing, std::size_t cycle);

    /**
     * @brief Closes the stream on demand and prevents any further operations.
     */
    void close();
};


#endif //RAMPACK_SIMULATIONRECORDER_H
