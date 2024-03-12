//
// Created by pkua on 18.11.22.
//

#include <iterator>

#include "XYZWriter.h"
#include "geometry/Quaternion.h"
#include "utils/Exceptions.h"


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

bool XYZWriter::isPackingPolydisperse(const Packing &packing) {
    auto dataDifferent = [](const Shape &s1, const Shape &s2) { return s1.getData() != s2.getData(); };
    return std::adjacent_find(packing.begin(), packing.end(), dataDifferent) != packing.end();
}

std::string XYZWriter::generateProceduralSpeciesName(const TextualShapeData &textualData) {
    std::ostringstream type;
    auto paramPrinter = [](const auto &param) { return param.first + ":" + param.second; };
    std::transform(textualData.begin(), textualData.end(), std::ostream_iterator<std::string>(type, "/"),
                   paramPrinter);
    std::string typeStr = type.str();
    typeStr.pop_back();
    return typeStr;
}

std::optional<std::string> XYZWriter::tryGetSpeciesName(const TextualShapeData &textualData) {
    if (textualData.size() != 1)
        return std::nullopt;

    if (textualData.begin()->first != "species")
        return std::nullopt;

    return textualData.begin()->second;
}

bool XYZWriter::isMapBijective(const SpeciesMap &speciesMap) {
    for (auto it1 = speciesMap.begin(); it1 != speciesMap.end(); it1++)
        for (auto it2 = std::next(it1); it2 != speciesMap.end(); it2++)
            if (it1->second == it2->second)
                return false;

    return true;
}

XYZWriter::XYZWriter(XYZWriter::SpeciesMap speciesMap) : speciesMap{std::move(speciesMap)} {
    Expects(XYZWriter::isMapBijective(this->speciesMap));
}

void XYZWriter::storeShapes(std::ostream &out, const Packing &packing, const ShapeDataManager &manager) const {
    bool isPolydisperse = XYZWriter::isPackingPolydisperse(packing);

    for (const auto &shape : packing) {
        std::string speciesName = this->findSpeciesName(manager, shape.getData(), isPolydisperse);
        const auto &pos = shape.getPosition();
        const auto &rot = shape.getOrientation();
        auto quat = Quaternion::fromMatrix(rot);
        out << speciesName << " ";
        out << pos[0] << " " << pos[1] << " " << pos[2] << " ";
        out << quat[0] << " " << quat[1] << " " << quat[2] << " " << quat[3] << std::endl;
    }
}

std::string XYZWriter::findSpeciesName(const ShapeDataManager &manager, const ShapeData &data,
                                       bool isPolydisperse) const
{
    auto textualData = manager.serialize(data);

    auto mappedSpeciesName = this->findMappedSpeciesName(data);
    if (mappedSpeciesName.has_value())
        return *mappedSpeciesName;

    auto speciesName = XYZWriter::tryGetSpeciesName(textualData);
    if (speciesName)
        return *speciesName;

    if (!isPolydisperse)
        return "A";

    return XYZWriter::generateProceduralSpeciesName(textualData);
}

std::optional<std::string> XYZWriter::findMappedSpeciesName(const ShapeData &data) const {
    for (const auto &[speciesName, speciesData] : this->speciesMap)
        if (speciesData == data)
            return speciesName;
    return std::nullopt;
}

void XYZWriter::write(std::ostream &out, const Packing &packing, const ShapeDataManager &manager,
                      const std::map<std::string, std::string> &auxInfo) const
{
    XYZWriter::storeHeader(out, packing, auxInfo);
    this->storeShapes(out, packing, manager);
}
