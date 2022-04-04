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
        std::size_t expectedPos = SimulationIO::posForSnapshot(header, this->numSnapshots);
        ValidateMsg(this->stream->tellp() == expectedPos, "RAMTRJ append error: broken snapshot structure");
    } else {
        this->stream->seekp(0, std::ios_base::end);
        ValidateMsg(this->stream->tellp() == 0, "RAMTRJ error: append = false however stream is not empty");

        // Write an uninitialized header to start with
        Header header;
        this->stream->write(reinterpret_cast<char*>(&header), sizeof(Header));
        ValidateMsg(*this->stream, "RAMTRJ write error: header");
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
    } else {
        Expects(cycle == (this->numSnapshots + 1) * this->cycleStep);
    }

    BoxData boxData;
    boxData.fromTriclinicBox(packing.getBox());
    this->stream->write(reinterpret_cast<char*>(&boxData), sizeof(BoxData));
    ValidateMsg(*this->stream, "RAMTRJ write error: shapshot box data");

    for (const auto &shape : packing) {
        ParticleData particleData;
        particleData.fromShape(shape);
        this->stream->write(reinterpret_cast<char*>(&particleData), sizeof(ParticleData));
        ValidateMsg(*this->stream, "RAMTRJ write error: shapshot particle data");
    }

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
    this->stream->write(reinterpret_cast<char*>(&header), sizeof(Header));
    ValidateMsg(*this->stream, "RAMTRJ write: header");

    this->stream = nullptr;
}
