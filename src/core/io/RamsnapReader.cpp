//
// Created by pkua on 18.11.22.
//

#include "RamsnapReader.h"
#include "utils/ParseUtils.h"


std::map<std::string, std::string> RamsnapReader::read(std::istream &in, Packing &packing,
                                                       const Interaction &interaction) const
{
    auto auxInfo = RamsnapReader::restoreAuxInfo(in);
    TriclinicBox box = RamsnapReader::restoreBox(in);
    std::vector<Shape> shapes = RamsnapReader::restoreShapes(in);
    packing.reset(std::move(shapes), box, interaction);
    return auxInfo;
}

std::vector<Shape> RamsnapReader::restoreShapes(std::istream &in) {
    std::size_t size{};
    in >> size;
    ValidateMsg(in, "Broken RAMSNAP file: size");

    std::vector<Shape> shapes;
    shapes.reserve(size);
    for (std::size_t i{}; i < size; i++) {
        Vector<3> position;
        Matrix<3, 3> orientation;
        in >> position[0] >> position[1] >> position[2];
        in >> orientation(0, 0) >> orientation(0, 1) >> orientation(0, 2);
        in >> orientation(1, 0) >> orientation(1, 1) >> orientation(1, 2);
        in >> orientation(2, 0) >> orientation(2, 1) >> orientation(2, 2);
        ValidateMsg(in, "Broken RAMSNAP file: shape " + std::to_string(shapes.size()) + "/"
                        + std::to_string(size));
        shapes.emplace_back(position, orientation);
    }
    return shapes;
}

std::map<std::string, std::string> RamsnapReader::restoreAuxInfo(std::istream &in) {
    std::size_t auxInfoSize{};
    in >> auxInfoSize;
    ValidateMsg(in, "Broken RAMSNAP file: aux info size");
    std::map<std::string, std::string> auxInfo;
    for (std::size_t i{}; i < auxInfoSize; i++) {
        std::string key;
        std::string value;
        in >> key >> std::ws;
        std::getline(in, value);
        ValidateMsg(in, "Broken RAMSNAP file: aux info entry " + std::to_string(i));
        auxInfo[key] = value;
    }

    return auxInfo;
}

TriclinicBox RamsnapReader::restoreBox(std::istream &in) {
    std::string line;
    std::getline(in, line);
    ValidateMsg(in, "Broken RAMSNAP file: dimensions");

    // We support both new and old format: in the old box was cuboidal, so only 3 numbers were stored. Now we store
    // a whole 9-element box matrix. The format can be recognized by the number of string in the line.
    double tokensOld[3];
    std::istringstream dimensionsStream(line);
    dimensionsStream >> tokensOld[0] >> tokensOld[1] >> tokensOld[2];
    ValidateMsg(dimensionsStream, "Broken RAMSNAP file: dimensions");

    Matrix<3, 3> dimensions;
    if (!ParseUtils::isAnythingLeft(dimensionsStream)) {
        // If eof, dimensions were saved in the old format: L_x, L_y, L_z
        dimensions(0, 0) = tokensOld[0];
        dimensions(1, 1) = tokensOld[1];
        dimensions(2, 2) = tokensOld[2];
    } else {            // Otherwise, new format - box matrix
        dimensions(0, 0) = tokensOld[0];
        dimensions(0, 1) = tokensOld[1];
        dimensions(0, 2) = tokensOld[2];
        dimensionsStream >> dimensions(1, 0) >> dimensions(1, 1) >> dimensions(1, 2);
        dimensionsStream >> dimensions(2, 0) >> dimensions(2, 1) >> dimensions(2, 2);
        ValidateMsg(dimensionsStream, "Broken RAMSNAP file: dimensions");
    }

    TriclinicBox box(dimensions);
    ValidateMsg(box.getVolume() != 0, "Broken RAMSNAP file: zero box volume");
    return box;
}