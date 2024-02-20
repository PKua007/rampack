//
// Created by pkua on 18.11.22.
//

#include "RamsnapReader.h"
#include "RamsnapIO.h"
#include "utils/ParseUtils.h"
#include "utils/Utils.h"


#define RamsnapValidateMsg(cond, msg) EXCEPTIONS_BLOCK(                                                             \
    if (!(cond))                                                                                                    \
        throw RamsnapException(msg);                                                                                \
)


auto RamsnapReader::restoreVersionAndAuxInfoSize(std::istream &in) {
    std::string line;
    RamsnapValidateMsg(RamsnapReader::getNonEmptyLine(in, line), "Broken RAMSNAP file: magic");
    std::istringstream magicOrAuxInfoIn(line);

    Version version;
    std::size_t auxInfoSize{};
    std::string magic;
    magicOrAuxInfoIn >> magic;
    if (magic == "RAMSNAP") {
        // New format:
        // RAMSNAP [major] [minor]
        // [aux info size]
        std::size_t major, minor;
        magicOrAuxInfoIn >> major >> minor;
        RamsnapValidateMsg(magicOrAuxInfoIn && !ParseUtils::isAnythingLeft(magicOrAuxInfoIn),
                           "Broken RAMSNAP file: version");
        version = Version(major, minor);
        RamsnapValidateMsg(version >= Version(1, 1), "Minimal RAMSNAP vesrion is 1.1; got " + version.str());

        in >> auxInfoSize;
        RamsnapValidateMsg(in, "Broken RAMSNAP file: aux info size");
    } else {
        // Old format (version assumed to be 1.0)
        // [aux info size]
        version = Version(1, 0);

        magicOrAuxInfoIn.seekg(0, std::ios::beg);
        magicOrAuxInfoIn >> auxInfoSize;
        RamsnapValidateMsg(magicOrAuxInfoIn && !ParseUtils::isAnythingLeft(magicOrAuxInfoIn),
                           "Broken RAMSNAP file: aux info size");
    }

    return std::make_pair(version, auxInfoSize);
}

auto RamsnapReader::restoreVersionAndAuxInfo(std::istream &in) {
    auto [version, auxInfoSize] = RamsnapReader::restoreVersionAndAuxInfoSize(in);

    std::map<std::string, std::string> params;
    for (std::size_t i{}; i < auxInfoSize; i++) {
        std::string key;
        std::string value;
        in >> key >> std::ws;
        std::getline(in, value);
        RamsnapValidateMsg(in, "Broken RAMSNAP file: aux info entry " + std::to_string(i));
        params[key] = value;
    }

    return std::make_pair(version, params);
}

TriclinicBox RamsnapReader::restoreBox(std::istream &in) {
    std::string line;
    RamsnapReader::getNonEmptyLine(in, line);
    RamsnapValidateMsg(in, "Broken RAMSNAP file: dimensions");

    // We support both new and old format: in the old box was cuboidal, so only 3 numbers were stored. Now we store
    // a whole 9-element box matrix. The format can be recognized by the number of string in the line.
    double tokensOld[3];
    std::istringstream dimensionsStream(line);
    dimensionsStream >> tokensOld[0] >> tokensOld[1] >> tokensOld[2];
    RamsnapValidateMsg(dimensionsStream, "Broken RAMSNAP file: dimensions");

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
        RamsnapValidateMsg(dimensionsStream, "Broken RAMSNAP file: dimensions");
    }

    TriclinicBox box(dimensions);
    RamsnapValidateMsg(box.getVolume() != 0, "Broken RAMSNAP file: zero box volume");
    return box;
}

std::vector<Shape> RamsnapReader::restoreShapes(std::istream &in, const ShapeDataManager &manager,
                                                const Version &version)
{
    std::size_t size{};
    in >> size;
    RamsnapValidateMsg(in, "Broken RAMSNAP file: number of shapes");

    std::vector<Shape> shapes;
    shapes.reserve(size);
    for (std::size_t shapeI{}; shapeI < size; shapeI++) {
        std::string line;
        RamsnapReader::getNonEmptyLine(in, line);
        RamsnapValidateMsg(in, RamsnapReader::generateBrokenShapeMsg(shapeI, size));
        std::istringstream shapeIn(line);

        Vector<3> position;
        Matrix<3, 3> orientation;
        shapeIn >> position[0] >> position[1] >> position[2];
        shapeIn >> orientation(0, 0) >> orientation(0, 1) >> orientation(0, 2);
        shapeIn >> orientation(1, 0) >> orientation(1, 1) >> orientation(1, 2);
        shapeIn >> orientation(2, 0) >> orientation(2, 1) >> orientation(2, 2);
        RamsnapValidateMsg(shapeIn, RamsnapReader::generateBrokenShapeMsg(shapeI, size, "position/orientation"));

        ShapeData data;
        if (version >= RamsnapIO::SHAPE_DATA_VERSION)
            data = RamsnapReader::restoreShapeData(shapeIn, shapeI, size, manager);

        shapes.emplace_back(position, orientation, data);
    }
    return shapes;
}

ShapeData RamsnapReader::restoreShapeData(std::istringstream &shapeIn, std::size_t shapeIdx, std::size_t shapeTotal,
                                          const ShapeDataManager &manager)
{
    std::size_t numParams;
    shapeIn >> numParams;
    RamsnapValidateMsg(shapeIn, RamsnapReader::generateBrokenShapeMsg(shapeIdx, shapeTotal, "shape data"));

    TextualShapeData textData;
    for (std::size_t paramI{}; paramI < numParams; paramI++) {
        std::string key, value;
        shapeIn >> key >> value;
        RamsnapValidateMsg(shapeIn, RamsnapReader::generateBrokenShapeMsg(shapeIdx, shapeTotal, "shape data"));
        auto containsQuotes = [](const std::string &str) { return str.find('"') != std::string::npos; };
        RamsnapValidateMsg(!containsQuotes(key),
                           RamsnapReader::generateBrokenShapeMsg(shapeIdx, shapeTotal, "shape data: quotes forbidden"));
        RamsnapValidateMsg(!containsQuotes(value),
                           RamsnapReader::generateBrokenShapeMsg(shapeIdx, shapeTotal, "shape data: quotes forbidden"));
        textData[key] = value;
    }

    ShapeData data;
    try {
        data = manager.defaultDeserialize(textData);
    } catch (const ShapeDataException &e) {
        auto details = std::string("shape data: ") + e.what();
        throw RamsnapException(RamsnapReader::generateBrokenShapeMsg(shapeIdx, shapeTotal, details));
    }
    return data;
}

bool RamsnapReader::getNonEmptyLine(std::istream &in, std::string &line) {
    while (std::getline(in, line))
        if (!containsOnlyWhitespace(line))
            return true;
    return false;
}

std::string RamsnapReader::generateBrokenShapeMsg(std::size_t shapeIdx, std::size_t shapeTotal,
                                                  const std::string &details)
{
    std::ostringstream msgOut;
    msgOut << "Broken RAMSNAP file: shape " << shapeIdx << "/" << shapeTotal;
    if (!details.empty())
        msgOut << ": " << details;
    return msgOut.str();
}

std::map<std::string, std::string> RamsnapReader::restoreAuxInfo(std::istream &in) {
    return RamsnapReader::restoreVersionAndAuxInfo(in).second;
}

std::map<std::string, std::string> RamsnapReader::read(std::istream &in, Packing &packing,
                                                       const Interaction &interaction,
                                                       const ShapeDataManager &manager) const
{
    auto [version, auxInfo] = RamsnapReader::restoreVersionAndAuxInfo(in);
    TriclinicBox box = RamsnapReader::restoreBox(in);
    std::vector<Shape> shapes = RamsnapReader::restoreShapes(in, manager, version);
    packing.reset(std::move(shapes), box, interaction, manager);
    return auxInfo;
}
