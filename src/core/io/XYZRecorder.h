//
// Created by pkua on 17.11.22.
//

#ifndef RAMPACK_XYZRECORDER_H
#define RAMPACK_XYZRECORDER_H

#include <ostream>

#include "core/SimulationRecorder.h"


class XYZRecorder : public SimulationRecorder {
private:
    std::unique_ptr<std::ostream> out;

public:
    explicit XYZRecorder(std::unique_ptr<std::ostream> out) : out{std::move(out)} { }
    ~XYZRecorder() override = default;

    void recordSnapshot(const Packing &packing, std::size_t cycle) override;

    void close() override { this->out = nullptr; }
};


#endif //RAMPACK_XYZRECORDER_H
