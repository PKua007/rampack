//
// Created by pkua on 04.04.2022.
//

#include "SimulationIO.h"
#include "utils/Assertions.h"
#include "geometry/EulerAngles.h"


SimulationIO::Header SimulationIO::readHeader(std::istream &stream) {
    Header header;
    stream.read(reinterpret_cast<char*>(&header), sizeof(header));
    ValidateMsg(stream, "RAMTRJ read error: header");
    ValidateMsg(std::string(&header.magic[0], &header.magic[6]) == "RAMTRJ", "RAMTRJ read error: magic");
    ValidateMsg(header.newline == '\n', "RAMTRJ read error: magic");
    ValidateMsg(header.versionMajor == 1 && header.versionMinor == 0, "RAMTRJ: only versions up to 1.0 are supported");
    ValidateMsg(header.numParticles > 0, "RAMTRJ read error: num particles");
    ValidateMsg(header.numSnapshots > 0, "RAMTRJ read error: num snapshots");
    ValidateMsg(header.cycleStep > 0, "RAMTRJ read error: cycle step");
    return header;
}

std::streamoff SimulationIO::posForSnapshot(const SimulationIO::Header &header, std::size_t snapshotNum) {
    std::size_t snapshotSize = sizeof(BoxData) + header.numParticles * sizeof(ParticleData);
    return sizeof(Header) + snapshotNum * snapshotSize;
}

TriclinicBox SimulationIO::BoxData::toTriclinicBox() const {
    Matrix<3, 3> boxMatrix(this->dimensions);
    return TriclinicBox(boxMatrix);
}

void SimulationIO::BoxData::fromTriclinicBox(const TriclinicBox &box) {
    box.getDimensions().copyToArray(this->dimensions);
}

Shape SimulationIO::ParticleData::toShape() const {
    Vector<3> position_(this->position);
    Matrix<3, 3> orientation = Matrix<3, 3>::rotation(this->eulerAngles[0], this->eulerAngles[1], this->eulerAngles[2]);
    return Shape(position_, orientation);
}

void SimulationIO::ParticleData::fromShape(const Shape &shape) {
    EulerAngles eulerAngles_(shape.getOrientation());
    std::copy(eulerAngles_.first.begin(), eulerAngles_.first.end(), std::begin(this->eulerAngles));
    shape.getPosition().copyToArray(this->position);
}
