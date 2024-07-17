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
 *
 * Each line representing a single shape starts with a particle type name. The name is generated using the following
 * decision tree:
 * - if SpeciesMap passed in the constructor contains a given particle's ShapeData, the corresponding name is used
 * - otherwise, if particle's serialized @ref TextualShapeData are a map with a single key `species`, its value is used
 *   as a name
 * - otherwise, if the Packing is monodisperse, `A` is used as a name (for backwards compatibility)
 * - finally, if any of the above fails to produce a name, it is generated procedurally from @ref TextualShapeData, with
 *   `[key]=[value]` pair formatted as `[key]:[value]`, joined by `/`. For example, for
 *   `TextualShapeData{{"r", "0.5"}, {"l", "1.5"}}` it would be `r=0.5/l=1.5`
 *
 * @sa
 * <a href="https://www.ovito.org/manual/reference/file_formats/input/xyz.html#file-formats-input-xyz-extended-format">
 *     Extended XYZ format
 * </a>
 */
class XYZWriter : public SnapshotWriter {
public:
    /**
     * @brief Mapping between the species name and corresponding ShapeData.
     */
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
    /**
     * @brief Creates the writer.
     * @param speciesMap an optional, custom mapping between the ShapeData and corresponding species names (see class
     * description)
     */
    explicit XYZWriter(SpeciesMap speciesMap = {});

    /**
     * @brief Writes the snapshot to @a out stream.
     * @details All stream flags of @a out are respected - the caller may use them to specify format and precision.
     * Particle type names (the first entry in each line) are generated as described in class description. Auxiliary
     * info @a auxInfo is stored in the header.
     */
    void write(std::ostream &out, const Packing &packing, const ShapeTraits &traits,
               const std::map<std::string, std::string> &auxInfo) const override
    {
        this->write(out, packing, traits.getDataManager(), auxInfo);
    }

    /**
     * @brief Convenient wrapper over
     * write(std::ostream&, const Packing&, const ShapeTraits&, const std::map<std::string, std::string>&) const
     * using only ShapeDataManager part of ShapeTraits.
     */
    void write(std::ostream &out, const Packing &packing, const ShapeDataManager &manager,
               const std::map<std::string, std::string> &auxInfo) const;
};


#endif //RAMPACK_XYZWRITER_H
