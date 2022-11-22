//
// Created by pkua on 18.11.22.
//

#include "RamsnapWriter.h"


void RamsnapWriter::write(std::ostream &out, const Packing &packing,
                          const std::map<std::string, std::string> &auxInfo) const
{
    out.precision(std::numeric_limits<double>::max_digits10);
    out << std::defaultfloat;
    RamsnapWriter::storeAuxInfo(out, auxInfo);
    RamsnapWriter::storeBox(out, packing.getBox());
    RamsnapWriter::storeShapes(out, packing);
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

void RamsnapWriter::storeShapes(std::ostream &out, const Packing &packing) {
    std::size_t size = packing.size();
    out << size << std::endl;
    for (const auto &shape : packing) {
        Vector<3> position = shape.getPosition();
        Matrix<3, 3> orientation = shape.getOrientation();
        out << position[0] << " " << position[1] << " " << position[2];
        out << "        ";
        out << orientation(0, 0) << " " << orientation(0, 1) << " " << orientation(0, 2) << " ";
        out << orientation(1, 0) << " " << orientation(1, 1) << " " << orientation(1, 2) << " ";
        out << orientation(2, 0) << " " << orientation(2, 1) << " " << orientation(2, 2);
        out << std::endl;
    }
}
