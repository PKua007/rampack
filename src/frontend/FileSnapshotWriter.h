//
// Created by Piotr Kubala on 18/01/2023.
//

#ifndef RAMPACK_FILESNAPSHOTWRITER_H
#define RAMPACK_FILESNAPSHOTWRITER_H

#include <string>
#include <utility>

#include "core/SnapshotWriter.h"
#include "core/Simulation.h"
#include "core/io/WolframWriter.h"
#include "core/io/RamsnapWriter.h"
#include "core/io/XYZWriter.h"


class FileSnapshotWriter {
private:
    std::string filename;
    std::string writerFormat;

    static std::string doubleToString(double d);
    static std::string formatMoveKey(const std::string &groupName, const std::string &moveName);
    static std::map<std::string, std::string> prepareAuxInfo(const Simulation &simulation);

    void doStore(const Packing &packing, const ShapeTraits &traits, const std::map<std::string, std::string> &auxInfo,
                 Logger &logger) const;

protected:
    [[nodiscard]] virtual std::shared_ptr<SnapshotWriter> createWriter(const ShapeTraits &traits) const = 0;

public:
    FileSnapshotWriter(std::string filename, std::string writerDescription)
        : filename{std::move(filename)}, writerFormat{std::move(writerDescription)}
    { }

    virtual ~FileSnapshotWriter() = default;

    [[nodiscard]] virtual bool storesRamsnap() const = 0;

    void generateSnapshot(const Packing &packing, const ShapeTraits &traits, std::size_t cycles, Logger &logger) const;
    void storeSnapshot(const Simulation &simulation, const ShapeTraits &traits, Logger &logger) const;
    [[nodiscard]] const std::string &getFilename() const { return this->filename; }
};


class WolframFileSnapshotWriter : public FileSnapshotWriter {
private:
    WolframWriter::WolframStyle style;
    std::map<std::string, std::string> params;

protected:
    [[nodiscard]] std::shared_ptr<SnapshotWriter> createWriter([[maybe_unused]] const ShapeTraits &traits) const override {
        return std::make_shared<WolframWriter>(this->style, this->params);
    }

public:
    WolframFileSnapshotWriter(std::string filename, WolframWriter::WolframStyle style,
                              std::map<std::string, std::string> params)
            : FileSnapshotWriter(std::move(filename), "Wolfram"), style{style},
              params{std::move(params)}
    { }

    [[nodiscard]] bool storesRamsnap() const override { return false; }
};


class RamsnapFileSnapshotWriter : public FileSnapshotWriter {
protected:
    [[nodiscard]] std::shared_ptr<SnapshotWriter> createWriter([[maybe_unused]] const ShapeTraits &traits) const override {
        return std::make_shared<RamsnapWriter>();
    }

public:
    explicit RamsnapFileSnapshotWriter(std::string filename) : FileSnapshotWriter(std::move(filename), "RAMSNAP") { }

    [[nodiscard]] bool storesRamsnap() const override { return true; }
};


class XYZFileSnapshotWriter : public FileSnapshotWriter {
private:
    std::map<std::string, TextualShapeData> textualSpeciesMap;

protected:
    [[nodiscard]] std::shared_ptr<SnapshotWriter> createWriter(const ShapeTraits &traits) const override;

public:
    explicit XYZFileSnapshotWriter(std::string filename, std::map<std::string, TextualShapeData> textualSpeciesMap = {})
            : FileSnapshotWriter(std::move(filename), "XYZ"), textualSpeciesMap{std::move(textualSpeciesMap)}
    { }

    [[nodiscard]] bool storesRamsnap() const override { return false; }
};



#endif //RAMPACK_FILESNAPSHOTWRITER_H
