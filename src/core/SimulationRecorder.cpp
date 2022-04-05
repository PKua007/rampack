//
// Created by pkua on 04.04.2022.
//

#include "SimulationRecorder.h"


SimulationRecorder::SimulationRecorder(std::unique_ptr<std::iostream> stream_, bool append) : stream{std::move(stream_)} {
    if (append) {
        this->stream->seekg(0);
        Header header = readHeader(*this->stream);

        this->cycleStep = header.cycleStep;
        this->numSnapshots = header.numSnapshots;
        this->numParticles = header.numParticles;

        this->stream->seekp(0, std::ios_base::end);
        std::streamoff expectedPos = SimulationIO::streamoffForSnapshot(header, this->numSnapshots);
        ValidateMsg(this->stream->tellp() == expectedPos, "RAMTRJ append error: broken snapshot structure");
    } else {
        this->stream->seekp(0, std::ios_base::end);
        ValidateMsg(this->stream->tellp() == 0, "RAMTRJ error: append = false however stream is not empty");

        // Write an uninitialized header to start with
        Header header;
        SimulationIO::writeHeader(header, *this->stream);
    }
}

SimulationRecorder::~SimulationRecorder() {
    this->close();
}

void SimulationRecorder::recordSnapshot(const Packing &packing, std::size_t cycle) {
    Expects(this->stream != nullptr);

    Expects(cycle > 0);
    if (this->numSnapshots == 0) {
        this->cycleStep = cycle;
        this->numParticles = packing.size();
    } else {
        Expects(cycle == (this->numSnapshots + 1) * this->cycleStep);
        Expects(packing.size() == this->numParticles);
    }

    SimulationIO::writeBox(packing.getBox(), *this->stream);
    for (const auto &shape : packing)
        SimulationIO::writeShape(shape, *this->stream);

    this->numSnapshots++;
}

void SimulationRecorder::close() {
    if (this->stream == nullptr)
        return;

    if (this->numSnapshots == 0)
        return;

    Header header;
    header.numSnapshots = this->numSnapshots;
    header.numParticles = this->numParticles;
    header.cycleStep = this->cycleStep;
    this->stream->seekp(0);
    SimulationIO::writeHeader(header, *this->stream);

    this->stream = nullptr;
}
