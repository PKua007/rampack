//
// Created by pkua on 17.11.22.
//

#ifndef RAMPACK_XYZRECORDER_H
#define RAMPACK_XYZRECORDER_H

#include <ostream>

#include "core/SimulationRecorder.h"


/**
 * @brief SimulationRecorder, using XYZWriter to store subsequent shapshots.
 * @details The snapshots contain information about triclinic simulation box and all shapes' positions and orientations.
 * Moreover, auxiliary info is stored in the XYZ header. The snapshots produced by this class are readily accepted by
 * <a href="https://www.ovito.org/">Ovito</a>.
 * @sa <a href="https://www.ovito.org/manual/reference/file_formats/input/xyz.html#file-formats-input-xyz">
 *     Extended XYZ format
 * </a>
 */
class XYZRecorder : public SimulationRecorder {
private:
    std::unique_ptr<std::ostream> out;

public:
    explicit XYZRecorder(std::unique_ptr<std::ostream> out) : out{std::move(out)} { }
    ~XYZRecorder() override = default;

    /**
     * @brief Writes the next snapshot.
     * @details @a cycle is stored in the snapshot header as a field named `cycles`.
     */
    void recordSnapshot(const Packing &packing, std::size_t cycle) override;

    void close() override { this->out = nullptr; }
};


#endif //RAMPACK_XYZRECORDER_H
