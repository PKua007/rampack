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


RamtrjIO::Header RamtrjIO::readHeaderAndShapeData(const ShapeDataManager &manager, std::istream &in) {
    Header header;
    in.read(reinterpret_cast<char*>(header.magic), sizeof(header.magic));
    RamtrjValidateMsg(in && std::string(&header.magic[0], &header.magic[7]) == "RAMTRJ\n",
                      "RAMTRJ read error: magic");
    in.read(reinterpret_cast<char*>(&header.versionMinor), sizeof(header.versionMinor));
    in.read(reinterpret_cast<char*>(&header.versionMajor), sizeof(header.versionMajor));
    RamtrjValidateMsg(in, "RAMTRJ read error: version");
    RamtrjValidateMsg(header.getVersion() <= CURRENT_VERSION,
                      "RAMTRJ: only versions up to " + header.getVersion().str() + " are supported");
    in.read(reinterpret_cast<char*>(&header.numParticles), sizeof(header.numParticles));
    RamtrjValidateMsg(in, "RAMTRJ read error: num particles");
    in.read(reinterpret_cast<char*>(&header.numSnapshots), sizeof(header.numSnapshots));
    RamtrjValidateMsg(in, "RAMTRJ read error: num snapshots");
    in.read(reinterpret_cast<char*>(&header.cycleStep), sizeof(header.cycleStep));
    RamtrjValidateMsg(in, "RAMTRJ read error: cycle step");

    if (header.getVersion() >= NONZERO_DATA_VERSION) {
        RamtrjValidateMsg(header.numParticles > 0, "RAMTRJ read error: num particles");
        RamtrjValidateMsg(header.cycleStep > 0, "RAMTRJ read error: cycle step");
    }

    if (header.getVersion() >= SHAPE_DATA_VERSION)
        header.shapeDatas = RamtrjIO::readShapeData(manager, header.numParticles, in);
    else
        header.shapeDatas.resize(header.numParticles, manager.defaultDeserialize({}));

    header.firstSnapshotOffset = in.tellg();

    return header;
}

void RamtrjIO::writeHeaderAndShapeData(Header &header, const ShapeDataManager &manager, std::ostream &out) {
    RamtrjIO::writeHeader(header, out);
    if (header.getVersion() >= SHAPE_DATA_VERSION)
        RamtrjIO::writeShapeData(header.shapeDatas, manager, out);

    header.firstSnapshotOffset = out.tellp();
    out.seekp(0);
    RamtrjIO::writeHeader(header, out);
    Assert(header.firstSnapshotOffset != Header::INVALID_OFFSET);
    out.seekp(header.firstSnapshotOffset);
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

std::vector<ShapeData> RamtrjIO::readShapeData(const ShapeDataManager &manager, std::size_t numParticles,
                                               std::istream &in)
{
    std::size_t numParams{};
    in.read(reinterpret_cast<char*>(&numParams), sizeof(numParams));
    RamtrjValidateMsg(in, "RAMTRJ read error: num shape params");

    try {
        if (numParams == 0)
            return std::vector<ShapeData>(numParams, manager.deserialize({}));

        std::vector<std::string> paramKeys;
        paramKeys.reserve(numParams);
        for (std::size_t i{}; i < numParams; i++)
            paramKeys.push_back(RamtrjIO::readWhitespaceDelimitedString(in));

        std::vector<ShapeData> shapeDatas;
        shapeDatas.reserve(numParticles);
        TextualShapeData textualData;
        for (std::size_t i{}; i < numParticles; i++) {
            for (std::size_t j{}; j < numParams; j++) {
                const auto &key = paramKeys[j];
                std::string value = RamtrjIO::readWhitespaceDelimitedString(in);
                textualData[key] = value;
            }
            shapeDatas.push_back(manager.deserialize(textualData));
        }

        return shapeDatas;
    } catch (const RamtrjIO &) {
        throw RamtrjException("RAMTRJ read error: shape params");
    } catch (const ShapeDataException &e) {
        throw RamtrjException(std::string("RAMTRJ read error: shape params format: ") + e.what());
    }
}

void RamtrjIO::writeShapeData(const std::vector<ShapeData> &shapeDatas, const ShapeDataManager &manager,
                              std::ostream &out)
{
    Expects(!shapeDatas.empty());

    TextualShapeData trialData = manager.serialize(shapeDatas.front());
    std::size_t numParams = trialData.size();
    out.write(reinterpret_cast<const char*>(&numParams), sizeof(numParams));
    RamtrjValidateMsg(out, "RAMTRJ write error: num shape params");

    try {
        for (const auto &[key, value]: trialData)
            RamtrjIO::writeSpaceDelimitedString(key, out);

        for (const auto &shapeData: shapeDatas)
            for (const auto &[key, value]: manager.serialize(shapeData))
                RamtrjIO::writeSpaceDelimitedString(value, out);
    } catch (const RamtrjIO &) {
        throw RamtrjException("RAMTRJ read error: shape params");
    }
}

std::string RamtrjIO::readWhitespaceDelimitedString(std::istream &in) {
    char byte{};
    std::ostringstream ostr;
    while (in.get(byte)) {
        if (std::isspace(byte))
            return ostr.str();
        ostr << byte;
    }
    throw RamtrjException("RAMTRJ read error: space delimited string");
}

void RamtrjIO::writeSpaceDelimitedString(const std::string &str, std::ostream &out) {
    Expects(std::none_of(str.begin(), str.end(), [](char c) -> bool { return std::isspace(c); }));
    out.write(str.c_str(), static_cast<std::streamsize>(str.size()));
    out.put(' ');
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

Shape RamtrjIO::readShape(const Header &header, std::size_t particleIdx, std::istream &in) {
    Expects(particleIdx < header.numParticles);

    double position_[3];
    double eulerAngles_[3];
    in.read(reinterpret_cast<char*>(position_), sizeof(position_));
    in.read(reinterpret_cast<char*>(eulerAngles_), sizeof(eulerAngles_));
    RamtrjValidateMsg(in, "RAMTRJ read error: snapshot particle data");

    Vector<3> position(position_);
    Matrix<3, 3> orientation = Matrix<3, 3>::rotation(eulerAngles_[0], eulerAngles_[1], eulerAngles_[2]);
    return Shape{position, orientation, header.shapeDatas[particleIdx]};
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
    Expects(header.firstSnapshotOffset != Header::INVALID_OFFSET);
    return static_cast<std::streamoff>(header.firstSnapshotOffset + snapshotNum * RamtrjIO::getSnapshotSize(header));
}

std::size_t RamtrjIO::getHeaderSize() {
    return sizeof(Header::magic) + sizeof(Header::versionMinor) + sizeof(Header::versionMajor)
           + sizeof(Header::numParticles) + sizeof(Header::numSnapshots) + sizeof(Header::cycleStep);
}

std::size_t RamtrjIO::getSnapshotSize(const RamtrjIO::Header &header) {
    std::size_t particleSize = 3*sizeof(double) + 3*sizeof(double);
    return 9*sizeof(double) + header.numParticles * particleSize;
}
