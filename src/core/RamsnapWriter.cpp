//
// Created by pkua on 18.11.22.
//

#include "RamsnapWriter.h"


void RamsnapWriter::write(std::ostream &out, const Packing &packing, [[maybe_unused]] const ShapeTraits &traits,
                          const std::map<std::string, std::string> &auxInfo) const
{
    out << auxInfo.size() << std::endl;
    for (const auto &infoEntry : auxInfo) {
        const auto &key = infoEntry.first;
        const auto &value = infoEntry.second;
        Expects(std::none_of(key.begin(), key.end(), [](char c) { return std::isspace(c); }));
        out << key << " " << value << std::endl;
    }

    const auto &box = packing.getBox();
    std::size_t size = packing.size();

    out.precision(std::numeric_limits<double>::max_digits10);
    const auto &dimensions = box.getDimensions();
    out << dimensions(0, 0) << " " << dimensions(0, 1) << " " << dimensions(0, 2) << " ";
    out << dimensions(1, 0) << " " << dimensions(1, 1) << " " << dimensions(1, 2) << " ";
    out << dimensions(2, 0) << " " << dimensions(2, 1) << " " << dimensions(2, 2) << std::endl;
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
