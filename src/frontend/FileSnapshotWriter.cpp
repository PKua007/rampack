//
// Created by Piotr Kubala on 18/01/2023.
//

#include <fstream>
#include <iterator>
#include <algorithm>

#include "FileSnapshotWriter.h"


void FileSnapshotWriter::doStore(const Packing &packing, const ShapeTraits &traits,
                                 const std::map<std::string, std::string> &auxInfo, Logger &logger) const
{
    std::ofstream out(this->filename);
    ValidateOpenedDesc(out, this->filename, "to store " + this->writerFormat + " snapshot data");
    this->createWriter(traits)->write(out, packing, traits, auxInfo);
    logger.info() << this->writerFormat << " snapshot data stored to '" << this->filename << "'" << std::endl;
}

void FileSnapshotWriter::generateSnapshot(const Packing &packing, const ShapeTraits &traits, std::size_t cycles,
                                          Logger &logger) const
{
    std::map<std::string, std::string> auxInfo;
    auxInfo["cycles"] = std::to_string(cycles);
    this->doStore(packing, traits, auxInfo, logger);
}

void FileSnapshotWriter::storeSnapshot(const Simulation &simulation, const ShapeTraits &traits, Logger &logger) const {
    this->doStore(simulation.getPacking(), traits, FileSnapshotWriter::prepareAuxInfo(simulation), logger);
}

std::string FileSnapshotWriter::doubleToString(double d) {
    std::ostringstream ostr;
    ostr.precision(std::numeric_limits<double>::max_digits10);
    ostr << d;
    return ostr.str();
}

std::string FileSnapshotWriter::formatMoveKey(const std::string &groupName, const std::string &moveName) {
    std::string moveKey = "step.";
    moveKey += groupName;
    moveKey += ".";
    moveKey += moveName;
    return moveKey;
}

std::map<std::string, std::string> FileSnapshotWriter::prepareAuxInfo(const Simulation &simulation) {
    std::map<std::string, std::string> auxInfo;

    auxInfo["cycles"] = std::to_string(simulation.getTotalCycles());

    const auto &movesStatistics = simulation.getMovesStatistics();
    for (const auto &moveStatistics: movesStatistics) {
        auto groupName = moveStatistics.groupName;
        for (const auto &stepSizeData: moveStatistics.stepSizeDatas) {
            std::string moveKey = FileSnapshotWriter::formatMoveKey(groupName, stepSizeData.moveName);
            auxInfo[moveKey] = doubleToString(stepSizeData.stepSize);
        }
    }

    return auxInfo;
}

std::shared_ptr<SnapshotWriter> XYZFileSnapshotWriter::createWriter(const ShapeTraits &traits) const {
    XYZWriter::SpeciesMap speciesMap;
    const auto &manager = traits.getDataManager();
    auto deserialize = [&manager, this](const std::pair<std::string, TextualShapeData> &textualEntry) {
        try {
            return std::make_pair(textualEntry.first, manager.defaultDeserialize(textualEntry.second));
        } catch (const ShapeDataException &e) {
            throw ValidationException("Could not store '" + this->getFilename()
                                      + "' XYZ snapshot: parsing params for species '" + textualEntry.first
                                      + "' failed: " + e.what());
        }
    };
    std::transform(this->textualSpeciesMap.begin(), this->textualSpeciesMap.end(),
                   std::inserter(speciesMap, speciesMap.end()), deserialize);

    return std::make_shared<XYZWriter>(std::move(speciesMap));
}
