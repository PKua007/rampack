//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_RAMSNAPREADER_H
#define RAMPACK_RAMSNAPREADER_H

#include "core/SnapshotReader.h"
#include "utils/Exceptions.h"
#include "utils/Version.h"


class RamsnapException : public ValidationException {
public:
    using ValidationException::ValidationException;
};


/**
 * @brief Read the RAMSNAP snapshot (see RamsnapWriter for file specification)
 */
class RamsnapReader : public SnapshotReader {
private:
    static auto restoreVersionAndAuxInfoSize(std::istream &in);
    static auto restoreVersionAndAuxInfo(std::istream &in);
    static TriclinicBox restoreBox(std::istream &in);
    static std::vector<Shape> restoreShapes(std::istream &in, const ShapeDataManager &manager, const Version &version);
    static ShapeData restoreShapeData(std::istringstream &shapeIn, size_t shapeIdx, size_t shapeTotal,
                                      const ShapeDataManager &manager);
    static bool getNonEmptyLine(std::istream &in, std::string &line);
    static std::string generateBrokenShapeMsg(std::size_t shapeIdx, std::size_t shapeTotal,
                                              const std::string &details = "");

public:
    /**
     * @brief Auxiliary method restoring only auxiliary information from the stream.
     */
    static std::map<std::string, std::string> restoreAuxInfo(std::istream &in);

    std::map<std::string, std::string> read(std::istream &in, Packing &packing,
                                            const ShapeTraits &traits) const override
    {
        return this->read(in, packing, traits.getInteraction(), traits.getDataManager());
    }

    /**
     * @brief Convenient wrapper over read(std::istream&, Packing&, const ShapeTraits&) const function with
     * Interaction and ShapeManager instead of the whole ShapeTraits on the arguments list
     */
    std::map<std::string, std::string> read(std::istream &in, Packing &packing, const Interaction &interaction,
                                            const ShapeDataManager &manager) const;
};


#endif //RAMPACK_RAMSNAPREADER_H
