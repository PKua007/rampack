//
// Created by pkua on 18.11.22.
//

#include "RamsnapWriter.h"
#include "RamsnapIO.h"
#include "utils/Utils.h"


void RamsnapWriter::write(std::ostream &out, const Packing &packing, const std::map<std::string, std::string> &auxInfo,
                          const ShapeDataManager &manager) const
{
    out.precision(std::numeric_limits<double>::max_digits10);
    out << std::defaultfloat;
    RamsnapWriter::storeMagicAndVersion(out);
    RamsnapWriter::storeAuxInfo(out, auxInfo);
    RamsnapWriter::storeBox(out, packing.getBox());
    RamsnapWriter::storeShapes(out, packing, manager);
}

void RamsnapWriter::storeMagicAndVersion(std::ostream &out) {
    out << "RAMSNAP " << RamsnapIO::CURRENT_VERSION.getMajor() << " " << RamsnapIO::CURRENT_VERSION.getMinor();
    out << std::endl;
}

void RamsnapWriter::storeAuxInfo(std::ostream &out, const std::map<std::string, std::string> &auxInfo) {
    out << auxInfo.size() << std::endl;
    for (const auto &infoEntry : auxInfo) {
        const auto &key = infoEntry.first;
        const auto &value = infoEntry.second;
        Expects(std::none_of(key.begin(), key.end(), [](char c) { return isspace(c); }));
        out << key << " " << value << std::endl;
    }
}

void RamsnapWriter::storeBox(std::ostream &out, const TriclinicBox &box) {
    const auto &dimensions = box.getDimensions();
    out << dimensions(0, 0) << " " << dimensions(0, 1) << " " << dimensions(0, 2) << " ";
    out << dimensions(1, 0) << " " << dimensions(1, 1) << " " << dimensions(1, 2) << " ";
    out << dimensions(2, 0) << " " << dimensions(2, 1) << " " << dimensions(2, 2) << std::endl;
}

void RamsnapWriter::storeShapes(std::ostream &out, const Packing &packing, const ShapeDataManager &manager) {
    std::size_t size = packing.size();
    out << size << std::endl;
    for (const auto &shape : packing) {
        const Vector<3> &position = shape.getPosition();
        const Matrix<3, 3> &orientation = shape.getOrientation();
        const ShapeData &data = shape.getData();
        out << position[0] << " " << position[1] << " " << position[2];
        out << "        ";
        out << orientation(0, 0) << " " << orientation(0, 1) << " " << orientation(0, 2) << " ";
        out << orientation(1, 0) << " " << orientation(1, 1) << " " << orientation(1, 2) << " ";
        out << orientation(2, 0) << " " << orientation(2, 1) << " " << orientation(2, 2);
        out << "        ";
        RamsnapWriter::storeShapeData(out, data, manager);
        out << std::endl;
    }
}

void RamsnapWriter::storeShapeData(std::ostream &out, const ShapeData &data, const ShapeDataManager &manager) {
    TextualShapeData textData = manager.serialize(data);
    out << textData.size();
    for (const auto &[key, val] : textData) {
        // Whitespaces and quotes aren't allowed
        auto containsQuotes = [](const std::string &str) { return str.find('"') != std::string::npos; };
        Expects(!containsWhitespace(key) && !containsQuotes(key));
        Expects(!containsWhitespace(val) && !containsQuotes(val));
        out << " " << key << " " << val;
    }
}
