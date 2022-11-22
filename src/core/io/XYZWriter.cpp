//
// Created by pkua on 18.11.22.
//

#include "XYZWriter.h"
#include "geometry/Quaternion.h"


void XYZWriter::write(std::ostream &out, const Packing &packing,
                      const std::map<std::string, std::string> &auxInfo) const
{
    XYZWriter::storeHeader(out, packing, auxInfo);
    XYZWriter::storeShapes(out, packing);
}

void XYZWriter::storeHeader(std::ostream &out, const Packing &packing,
                            const std::map<std::string, std::string> &auxInfo)
{
    std::size_t numMolecules = packing.size();
    const auto &boxMatrix = packing.getBox().getDimensions();

    out << numMolecules << std::endl;
    out << "Lattice=\"";
    out << boxMatrix(0, 0) << " " << boxMatrix(1, 0) << " " << boxMatrix(2, 0) << " ";
    out << boxMatrix(0, 1) << " " << boxMatrix(1, 1) << " " << boxMatrix(2, 1) << " ";
    out << boxMatrix(0, 2) << " " << boxMatrix(1, 2) << " " << boxMatrix(2, 2);
    out << "\" Properties=species:S:1:pos:R:3:orientation:R:4";
    XYZWriter::storeAuxInfo(out, auxInfo);
    out << std::endl;
}

void XYZWriter::storeAuxInfo(std::ostream &out, const std::map<std::string, std::string> &auxInfo) {
    for (const auto &[key, value] : auxInfo) {
        auto hasWhitespace = [](const std::string &str) {
            return std::any_of(str.begin(), str.end(), [](char c) -> bool { return isspace(c); });
        };
        Assert(!hasWhitespace(key));
        out << " " << key << "=";
        if (hasWhitespace(value))
            out << '"' << value << '"';
        else
            out << value;
    }
}

void XYZWriter::storeShapes(std::ostream &out, const Packing &packing) {
    for (const auto &shape : packing) {
        const auto &pos = shape.getPosition();
        const auto &rot = shape.getOrientation();
        auto quat = Quaternion::fromMatrix(rot);
        out << "A ";
        out << pos[0] << " " << pos[1] << " " << pos[2] << " ";
        out << quat[0] << " " << quat[1] << " " << quat[2] << " " << quat[3] << std::endl;
    }
}
