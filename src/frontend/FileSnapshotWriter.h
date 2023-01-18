//
// Created by Piotr Kubala on 18/01/2023.
//

#ifndef RAMPACK_FILESNAPSHOTWRITER_H
#define RAMPACK_FILESNAPSHOTWRITER_H

#include <string>

#include "core/SnapshotWriter.h"
#include "core/Simulation.h"


class FileSnapshotWriter {
private:
    std::string filename;
    std::string writerFormat;
    std::shared_ptr<SnapshotWriter> writer;

    static std::string doubleToString(double d);
    static std::string formatMoveKey(const std::string &groupName, const std::string &moveName);
    static std::map<std::string, std::string> prepareAuxInfo(const Simulation &simulation);

    void doStore(const Packing &packing, const ShapeTraits &traits, const std::map<std::string, std::string> &auxInfo,
                 Logger &logger) const;

public:
    FileSnapshotWriter(std::string filename, std::string writerDescription,
                       const std::shared_ptr<SnapshotWriter> &writer)
        : filename{std::move(filename)}, writerFormat{std::move(writerDescription)}, writer{writer}
    { }

    void generateSnapshot(const Packing &packing, const ShapeTraits &traits, std::size_t cycles, Logger &logger) const;
    void storeSnapshot(const Simulation &simulation, const ShapeTraits &traits, Logger &logger) const;
};

#endif //RAMPACK_FILESNAPSHOTWRITER_H
