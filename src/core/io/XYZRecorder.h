//
// Created by pkua on 17.11.22.
//

#ifndef RAMPACK_XYZRECORDER_H
#define RAMPACK_XYZRECORDER_H

#include <ostream>

#include "core/SimulationRecorder.h"
#include "XYZWriter.h"


/**
 * @brief SimulationRecorder storing snapshots in
 * <a href="https://www.ovito.org/manual/reference/file_formats/input/xyz.html#file-formats-input-xyz-extended-format">
 *     Extended XYZ
 * </a>
 * format. Internally, XYZWriter is used to store each snapshot (see the documentation therein for the details).
 */
class XYZRecorder : public SimulationRecorder {
private:
    XYZWriter writer;
    std::unique_ptr<std::iostream> out;
    std::size_t lastCycleNumber{};

    void findLastCycleNumber();

public:
    /**
     * @brief Creates the recorder.
     * @param out i/o text stream to store the trajectory
     * @param append determines the recording mode:
     * - if `true`, it signals that the stream contains previously recorded snapshots and further ones will be appended.
     *   Last snapshot cycle number registered in the stream is correctly returned by getLastCycleNumber()
     * - if `false`, it signals that the stream should be empty and the trajectory will be recorded from scratch
     * @param speciesMap species map passed to underlying XYZWriter (see the documentation therein)
     * @throws PreconditionException if @a out is `nullptr`, or @a append is `false`, but @a out is not empty
     */
    explicit XYZRecorder(std::unique_ptr<std::iostream> out, bool append, XYZWriter::SpeciesMap speciesMap = {});

    ~XYZRecorder() override = default;

    /**
     * @brief Writes the next snapshot.
     * @details @a cycle is stored in the snapshot header as a field named `cycles`.
     */
    void recordSnapshot(const Packing &packing, const ShapeTraits &traits, std::size_t cycle) override;

    [[nodiscard]] std::size_t getLastCycleNumber() const override { return this->lastCycleNumber; }
    void close() override { this->out = nullptr; }
};


#endif //RAMPACK_XYZRECORDER_H
