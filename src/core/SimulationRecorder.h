//
// Created by pkua on 17.11.22.
//

#ifndef RAMPACK_SIMULATIONRECORDER_H
#define RAMPACK_SIMULATIONRECORDER_H

#include "Packing.h"


/**
 * @brief Class recording simulation trajectory.
 * @details The format and sink for the trajectory is implementation-specific.
 */
class SimulationRecorder {
public:
    virtual ~SimulationRecorder() = default;

    /**
     * @brief Records the next snapshot.
     */
    virtual void recordSnapshot(const Packing &packing, std::size_t cycle) = 0;

    /**
     * @brief Returns last recorder cycle number.
     */
     [[nodiscard]] virtual std::size_t getLastCycleNumber() const = 0;

    /**
     * @brief Closes the underlying sink on demand and prevents any further operations.
     */
    virtual void close() = 0;
};


#endif //RAMPACK_SIMULATIONRECORDER_H
