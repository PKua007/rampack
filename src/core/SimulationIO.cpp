//
// Created by pkua on 04.04.2022.
//

#include "SimulationIO.h"
#include "utils/Assertions.h"
#include "geometry/EulerAngles.h"


SimulationIO::Header SimulationIO::readHeader(std::istream &stream) {
    Header header;
    stream.read(reinterpret_cast<char*>(header.magic), sizeof(header.magic));
    ValidateMsg(stream && std::string(&header.magic[0], &header.magic[7]) == "RAMTRJ\n", "RAMTRJ read error: magic");
    stream.read(reinterpret_cast<char*>(&header.versionMinor), sizeof(header.versionMinor));
    stream.read(reinterpret_cast<char*>(&header.versionMajor), sizeof(header.versionMajor));
    ValidateMsg(stream && header.versionMajor == 1 && header.versionMinor == 0,
                "RAMTRJ: only versions up to 1.0 are supported");
    stream.read(reinterpret_cast<char*>(&header.numParticles), sizeof(header.numParticles));
    ValidateMsg(stream && header.numParticles > 0, "RAMTRJ read error: num particles");
    stream.read(reinterpret_cast<char*>(&header.numSnapshots), sizeof(header.numSnapshots));
    ValidateMsg(stream && header.numSnapshots > 0, "RAMTRJ read error: num snapshots");
    stream.read(reinterpret_cast<char*>(&header.cycleStep), sizeof(header.cycleStep));
    ValidateMsg(stream && header.cycleStep > 0, "RAMTRJ read error: cycle step");
    return header;
}

void SimulationIO::writeHeader(const Header &header, std::ostream &stream) {
    stream.write(reinterpret_cast<const char*>(header.magic), sizeof(header.magic));
    stream.write(reinterpret_cast<const char*>(&header.versionMinor), sizeof(header.versionMinor));
    stream.write(reinterpret_cast<const char*>(&header.versionMajor), sizeof(header.versionMajor));
    stream.write(reinterpret_cast<const char*>(&header.numParticles), sizeof(header.numParticles));
    stream.write(reinterpret_cast<const char*>(&header.numSnapshots), sizeof(header.numSnapshots));
    stream.write(reinterpret_cast<const char*>(&header.cycleStep), sizeof(header.cycleStep));
    ValidateMsg(stream, "RAMTRJ write error: header");
}

std::streamoff SimulationIO::posForSnapshot(const SimulationIO::Header &header, std::size_t snapshotNum) {
    std::size_t particleSize = sizeof(ParticleData::position) + sizeof(ParticleData::eulerAngles);
    std::size_t snapshotSize = sizeof(BoxData::dimensions) + header.numParticles * particleSize;
    return static_cast<std::streamoff>(SimulationIO::getHeaderSize() + snapshotNum * snapshotSize);
}

std::size_t SimulationIO::getHeaderSize() {
    return sizeof(Header::magic) + sizeof(Header::versionMinor) + sizeof(Header::versionMajor)
           + sizeof(Header::numParticles) + sizeof(Header::numSnapshots) + sizeof(Header::cycleStep);
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
