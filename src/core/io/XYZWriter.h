//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_XYZWRITER_H
#define RAMPACK_XYZWRITER_H

#include "core/SnapshotWriter.h"


/**
 * @brief SnapshotWriter storing packing in an (extended) XYZ format.
 * @details The snapshot contains information about triclinic simulation box and all shapes' positions and orientations.
 * Moreover, auxiliary info is stored in the XYZ header. The snapshots produced by this class are readily accepted by
 * <a href="https://www.ovito.org/">Ovito</a>.
 * @sa <a href="https://www.ovito.org/manual/reference/file_formats/input/xyz.html#file-formats-input-xyz">
 *     Extended XYZ format
 * </a>
 */
class XYZWriter : public SnapshotWriter {
public:
    using SpeciesMap = std::map<std::string, ShapeData>;

private:
    static void storeHeader(std::ostream &out, const Packing &packing,
                            const std::map<std::string, std::string> &auxInfo);
    static void storeAuxInfo(std::ostream &out, const std::map<std::string, std::string> &auxInfo);
    [[nodiscard]] static bool isPackingPolydisperse(const Packing &packing);
    [[nodiscard]] static std::string generateProceduralSpeciesName(const TextualShapeData &textualData);
    [[nodiscard]] static std::optional<std::string> tryGetSpeciesName(const TextualShapeData &textualData);

    void storeShapes(std::ostream &out, const Packing &packing, const ShapeDataManager &manager) const;
    [[nodiscard]] std::string findSpeciesName(const ShapeDataManager &manager, const ShapeData &data,
                                              bool isPolydisperse) const;
    [[nodiscard]] std::optional<std::string> findMappedSpeciesName(const ShapeData &data) const;

    SpeciesMap speciesMap;

public:
    explicit XYZWriter(SpeciesMap speciesMap = {});

    /**
     * @brief Writes the snapshot to @a out stream.
     * @details All stream flag of @a out are respected - the caller may use them to specify format and precision.
     */
    void write(std::ostream &out, const Packing &packing, const ShapeTraits &traits,
               const std::map<std::string, std::string> &auxInfo) const override
    {
        this->write(out, packing, traits.getDataManager(), auxInfo);
    }

    /**
     * @brief Convenient wrapper over
     * write(std::ostream&, const Packing&, const ShapeTraits&, const std::map<std::string, std::string>&) const
     * without unused ShapeTraits on the argument list.
     */
    void write(std::ostream &out, const Packing &packing, const ShapeDataManager &manager,
               const std::map<std::string, std::string> &auxInfo) const;
};


#endif //RAMPACK_XYZWRITER_H
