//
// Created by Piotr Kubala on 18/01/2023.
//

#include <fstream>

#include "FileSnapshotWriter.h"


void FileSnapshotWriter::doStore(const Packing &packing, const ShapeTraits &traits,
                                 const std::map<std::string, std::string> &auxInfo, Logger &logger) const
{
    std::ofstream out(this->filename);
    ValidateOpenedDesc(out, this->filename, "to store " + this->writerFormat + " snapshot data");
    this->writer->write(out, packing, traits, auxInfo);
    logger.info() << this->writerFormat << " snapshot data stored to " << this->filename << std::endl;
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
    for (const auto &moveStatistics : movesStatistics) {
        auto groupName = moveStatistics.groupName;
        for (const auto &stepSizeData : moveStatistics.stepSizeDatas) {
            std::string moveKey = FileSnapshotWriter::formatMoveKey(groupName, stepSizeData.moveName);
            auxInfo[moveKey] = doubleToString(stepSizeData.stepSize);
        }
    }

    return auxInfo;
}
