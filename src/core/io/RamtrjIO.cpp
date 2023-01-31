//
// Created by pkua on 04.04.2022.
//

#include "RamtrjIO.h"
#include "utils/Exceptions.h"
#include "geometry/EulerAngles.h"


#define RamtrjValidateMsg(cond, msg) EXCEPTIONS_BLOCK(                                                              \
    if (!(cond))                                                                                                    \
        throw RamtrjException(msg);                                                                                 \
)


RamtrjIO::Header RamtrjIO::readHeader(std::istream &in) {
    Header header;
    in.read(reinterpret_cast<char*>(header.magic), sizeof(header.magic));
    RamtrjValidateMsg(in && std::string(&header.magic[0], &header.magic[7]) == "RAMTRJ\n",
                      "RAMTRJ read error: magic");
    in.read(reinterpret_cast<char*>(&header.versionMinor), sizeof(header.versionMinor));
    in.read(reinterpret_cast<char*>(&header.versionMajor), sizeof(header.versionMajor));
    RamtrjValidateMsg(in && header.versionMajor == 1 && header.versionMinor <= 1,
                      "RAMTRJ: only versions up to 1.1 are supported");
    in.read(reinterpret_cast<char*>(&header.numParticles), sizeof(header.numParticles));
    RamtrjValidateMsg(in, "RAMTRJ read error: num particles");
    in.read(reinterpret_cast<char*>(&header.numSnapshots), sizeof(header.numSnapshots));
    RamtrjValidateMsg(in, "RAMTRJ read error: num snapshots");
    in.read(reinterpret_cast<char*>(&header.cycleStep), sizeof(header.cycleStep));
    RamtrjValidateMsg(in, "RAMTRJ read error: cycle step");

    if (header.versionMajor >= 1 && header.versionMinor >= 1) {
        RamtrjValidateMsg(header.numParticles > 0, "RAMTRJ read error: num particles");
        RamtrjValidateMsg(header.cycleStep > 0, "RAMTRJ read error: cycle step");
    }

    return header;
}

void RamtrjIO::writeHeader(const Header &header, std::ostream &out) {
    out.write(reinterpret_cast<const char*>(header.magic), sizeof(header.magic));
    out.write(reinterpret_cast<const char*>(&header.versionMinor), sizeof(header.versionMinor));
    out.write(reinterpret_cast<const char*>(&header.versionMajor), sizeof(header.versionMajor));
    out.write(reinterpret_cast<const char*>(&header.numParticles), sizeof(header.numParticles));
    out.write(reinterpret_cast<const char*>(&header.numSnapshots), sizeof(header.numSnapshots));
    out.write(reinterpret_cast<const char*>(&header.cycleStep), sizeof(header.cycleStep));
    RamtrjValidateMsg(out, "RAMTRJ write error: header");
}

TriclinicBox RamtrjIO::readBox(std::istream &in) {
    double dimensions_[9];
    in.read(reinterpret_cast<char*>(dimensions_), sizeof(dimensions_));
    RamtrjValidateMsg(in, "RAMTRJ read error: snapshot box data");
    return TriclinicBox(Matrix<3, 3>(dimensions_));
}

void RamtrjIO::writeBox(const TriclinicBox &box, std::ostream &out) {
    double dimensions_[9];
    box.getDimensions().copyToArray(dimensions_);
    out.write(reinterpret_cast<const char*>(dimensions_), sizeof(dimensions_));
    RamtrjValidateMsg(out, "RAMTRJ write error: shapshot box data");
}

Shape RamtrjIO::readShape(std::istream &in) {
    double position_[3];
    double eulerAngles_[3];
    in.read(reinterpret_cast<char*>(position_), sizeof(position_));
    in.read(reinterpret_cast<char*>(eulerAngles_), sizeof(eulerAngles_));
    RamtrjValidateMsg(in, "RAMTRJ read error: snapshot particle data");

    Vector<3> position(position_);
    Matrix<3, 3> orientation = Matrix<3, 3>::rotation(eulerAngles_[0], eulerAngles_[1], eulerAngles_[2]);
    return Shape{position, orientation};
}

void RamtrjIO::writeShape(const Shape &shape, std::ostream &out) {
    EulerAngles eulerAngles(shape.getOrientation());
    double eulerAngles_[3];
    std::copy(eulerAngles.first.begin(), eulerAngles.first.end(), std::begin(eulerAngles_));
    double position_[3];
    shape.getPosition().copyToArray(position_);
    out.write(reinterpret_cast<const char*>(position_), sizeof(position_));
    out.write(reinterpret_cast<const char*>(eulerAngles_), sizeof(eulerAngles_));
    RamtrjValidateMsg(out, "RAMTRJ write error: shapshot particle data");
}

std::streamoff RamtrjIO::streamoffForSnapshot(const RamtrjIO::Header &header, std::size_t snapshotNum) {
    return static_cast<std::streamoff>(
        RamtrjIO::getHeaderSize() + snapshotNum * RamtrjIO::getSnapshotSize(header)
    );
}

std::size_t RamtrjIO::getHeaderSize() {
    return sizeof(Header::magic) + sizeof(Header::versionMinor) + sizeof(Header::versionMajor)
           + sizeof(Header::numParticles) + sizeof(Header::numSnapshots) + sizeof(Header::cycleStep);
}

std::size_t RamtrjIO::getSnapshotSize(const RamtrjIO::Header &header) {
    std::size_t particleSize = 3*sizeof(double) + 3*sizeof(double);
    return 9*sizeof(double) + header.numParticles * particleSize;
}
