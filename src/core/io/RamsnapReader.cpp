//
// Created by pkua on 18.11.22.
//

#include "RamsnapReader.h"
#include "utils/ParseUtils.h"
#include "utils/Utils.h"


#define RamsnapValidateMsg(cond, msg) EXCEPTIONS_BLOCK(                                                             \
    if (!(cond))                                                                                                    \
        throw RamsnapException(msg);                                                                                \
)


std::map<std::string, std::string> RamsnapReader::read(std::istream &in, Packing &packing,
                                                       const Interaction &interaction,
                                                       const ShapeDataManager &manager) const
{
    auto auxInfo = RamsnapReader::restoreAuxInfo(in);
    TriclinicBox box = RamsnapReader::restoreBox(in);
    std::vector<Shape> shapes = RamsnapReader::restoreShapes(in, manager);
    packing.reset(std::move(shapes), box, interaction, manager);
    return auxInfo;
}

std::vector<Shape> RamsnapReader::restoreShapes(std::istream &in, const ShapeDataManager &manager) {
    std::size_t size{};
    in >> size;
    RamsnapValidateMsg(in, "Broken RAMSNAP file: number of shapes");

    std::vector<Shape> shapes;
    shapes.reserve(size);
    auto generateBrokenShapeMsg = [&shapes, size](const std::string &details = "") -> std::string {
        std::ostringstream msgOut;
        msgOut << "Broken RAMSNAP file: shape " << shapes.size() << "/" << size;
        if (!details.empty())
            msgOut << ": " << details;
        return msgOut.str();
    };
    for (std::size_t shapeI{}; shapeI < size; shapeI++) {
        std::string line;
        RamsnapReader::getNonEmptyLine(in, line);
        RamsnapValidateMsg(in, generateBrokenShapeMsg());
        std::istringstream shapeIn(line);

        Vector<3> position;
        Matrix<3, 3> orientation;
        shapeIn >> position[0] >> position[1] >> position[2];
        shapeIn >> orientation(0, 0) >> orientation(0, 1) >> orientation(0, 2);
        shapeIn >> orientation(1, 0) >> orientation(1, 1) >> orientation(1, 2);
        shapeIn >> orientation(2, 0) >> orientation(2, 1) >> orientation(2, 2);
        RamsnapValidateMsg(shapeIn, generateBrokenShapeMsg("position/orientation"));

        std::size_t numParams{};
        if (ParseUtils::isAnythingLeft(shapeIn)) {
            shapeIn >> numParams;
            RamsnapValidateMsg(shapeIn, generateBrokenShapeMsg("shape data"));
        }
        TextualShapeData textData;
        for (std::size_t paramI{}; paramI < numParams; paramI++) {
            std::string key, value;
            shapeIn >> key >> value;
            RamsnapValidateMsg(shapeIn, generateBrokenShapeMsg("shape data"));
            auto containsQuotes = [](const std::string &str) { return str.find('"') != std::string::npos; };
            RamsnapValidateMsg(!containsQuotes(key), generateBrokenShapeMsg("shape data: quotes forbidden"));
            RamsnapValidateMsg(!containsQuotes(value), generateBrokenShapeMsg("shape data: quotes forbidden"));
            textData[key] = value;
        }

        ShapeData data;
        try {
            data = manager.deserialize(textData);
        } catch (const ShapeDataException &e) {
            throw RamsnapException(generateBrokenShapeMsg(std::string("shape data: ") + e.what()));
        }

        shapes.emplace_back(position, orientation, data);
    }
    return shapes;
}

std::map<std::string, std::string> RamsnapReader::restoreAuxInfo(std::istream &in) {
    std::size_t auxInfoSize{};
    in >> auxInfoSize;
    RamsnapValidateMsg(in, "Broken RAMSNAP file: aux info size");
    std::map<std::string, std::string> auxInfo;
    for (std::size_t i{}; i < auxInfoSize; i++) {
        std::string key;
        std::string value;
        in >> key >> std::ws;
        std::getline(in, value);
        RamsnapValidateMsg(in, "Broken RAMSNAP file: aux info entry " + std::to_string(i));
        auxInfo[key] = value;
    }

    return auxInfo;
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

bool RamsnapReader::getNonEmptyLine(std::istream &in, std::string &line) {
    while (std::getline(in, line))
        if (!containsOnlyWhitespace(line))
            return true;
    return false;
}
