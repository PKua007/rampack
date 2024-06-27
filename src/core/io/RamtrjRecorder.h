//
// Created by pkua on 04.04.2022.
//

#ifndef RAMPACK_RAMTRJRECORDER_H
#define RAMPACK_RAMTRJRECORDER_H

#include <iostream>
#include <memory>

#include "core/Packing.h"
#include "RamtrjIO.h"
#include "core/SimulationRecorder.h"


/**
 * @brief A class which enables recording simulation to a binary format.
 */
class RamtrjRecorder : RamtrjIO, public SimulationRecorder {
private:
    std::unique_ptr<std::iostream> stream;
    Header header;

    void close0();

public:
    /**
     * @brief Constructs the recorder using a given @a std::iostream.
     * @details In the append mode (`append == true`), the class respects the original RAMTRJ version of the file.
     * Namely, after new snapshots are appended, the old RAMPACK version which originally generated the trajectory, will
     * be able to read it properly.
     * @param stream i/o stream where the trajectory will be stored. The class takes full responsibility of the stream.
     * It should be opened in binary input-output mode with all stream pointer methods working (@a tellp, @a seekp,
     * @a tellg, @a seekg)
     * @param packing packing, which will be recorded. It is used to determine the number of particles and store their
     * ShapeData
     * @param manager shape data manager to serialize the data
     * @param cycleStep interval between the cycle number of stored snapshots
     * @param append if @a true, new snapshots will be appended and it is assumed that the @a stream already contains
     * correct recording. If @a append is @a false, the stream should be empty, or else an error is reported.
     */
    RamtrjRecorder(std::unique_ptr<std::iostream> stream, const Packing &packing, const ShapeDataManager &manager,
                   std::size_t cycleStep, bool append);

    ~RamtrjRecorder() override;

    /**
     * @brief Records the next snapshot.
     * @details For each invocation, @a cycle should be a subsequent multiple of cycle step size (in agreement with
     * the one passed in the constructor). For example, subsequent invocations can be with @a cycle equal 200, 400,
     * 600, etc.
     */
    void recordSnapshot(const Packing &packing, const ShapeTraits &traits, std::size_t cycle) override;

    [[nodiscard]] std::size_t getLastCycleNumber() const override {
        return this->header.numSnapshots * this->header.cycleStep;
    }

    void close() override { this->close0(); }
};


#endif //RAMPACK_RAMTRJRECORDER_H
