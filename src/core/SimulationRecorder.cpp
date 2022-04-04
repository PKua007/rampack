//
// Created by pkua on 04.04.2022.
//

#include "SimulationRecorder.h"
#include "geometry/EulerAngles.h"


SimulationRecorder::SimulationRecorder(std::unique_ptr<std::iostream> stream_, bool append) : stream{std::move(stream_)} {
    if (append) {
        this->stream->seekg(0);
        Header header = readHeader(*this->stream);

        this->cycleStep = header.cycleStep;
        this->numSnapshots = header.numSnapshots;
        this->numParticles = header.numParticles;

        this->stream->seekp(0, std::ios_base::end);
        std::size_t snapshotSize = sizeof(BoxData) + this->numParticles * sizeof(ParticleData);
        std::size_t expectedPos = sizeof(Header) + this->numSnapshots * snapshotSize;
        ValidateMsg(this->stream->tellp() == expectedPos, "RAMTRJ append: broken snapshot structure");
    } else {
        this->stream->seekp(0, std::ios_base::end);
        ValidateMsg(this->stream->tellp() == 0, "RAMTRJ: append = false however stream is not empty");

        // Write an uninitialized header to start with
        Header header;
        this->stream->write(reinterpret_cast<char*>(&header), sizeof(Header));
        ValidateMsg(*this->stream, "RAMTRJ write: header");
    }
}

SimulationRecorder::~SimulationRecorder() {
    this->close();
}

SimulationRecorder::Header SimulationRecorder::readHeader(std::istream &stream) {
    Header header;
    stream.read(reinterpret_cast<char*>(&header), sizeof(header));
    ValidateMsg(stream, "RAMTRJ read: header");
    ValidateMsg(std::string(&header.magic[0], &header.magic[6]) == "RAMTRJ", "RAMTRJ read: magic");
    ValidateMsg(header.newline == '\n', "RAMTRJ read: magic");
    ValidateMsg(header.versionMajor == 1 && header.versionMinor == 0, "RAMTRJ: only versions up to 1.0 are supported");
    ValidateMsg(header.numParticles > 0, "RAMTRJ read: num particles");
    ValidateMsg(header.numSnapshots > 0, "RAMTRJ read: num snapshots");
    ValidateMsg(header.cycleStep > 0, "RAMTRJ read: cycle step");
    return header;
}

void SimulationRecorder::recordSnapshot(const Packing &packing, std::size_t cycle) {
    Expects(this->stream != nullptr);

    Expects(cycle > 0);
    if (this->numSnapshots == 0) {
        this->cycleStep = cycle;
    } else {
        Expects(cycle == (this->numSnapshots - 1) * this->cycleStep);
    }

    BoxData boxData;
    packing.getBox().getDimensions().copyToArray(boxData.dimensions);
    this->stream->write(reinterpret_cast<char*>(&boxData), sizeof(BoxData));
    ValidateMsg(*this->stream, "RAMTRJ write: shapshot box data");

    for (const auto &shape : packing) {
        EulerAngles eulerAngles(shape.getOrientation());
        ParticleData particleData;
        shape.getPosition().copyToArray(particleData.position);
        std::copy(eulerAngles.first.begin(), eulerAngles.first.end(), std::begin(particleData.eulerAngles));
        this->stream->write(reinterpret_cast<char*>(&particleData), sizeof(ParticleData));
        ValidateMsg(*this->stream, "RAMTRJ write: shapshot particle data");
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
