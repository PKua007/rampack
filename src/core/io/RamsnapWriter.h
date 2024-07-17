//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_RAMSNAPWRITER_H
#define RAMPACK_RAMSNAPWRITER_H

#include "core/SnapshotWriter.h"


/**
 * @brief Stores the snapshot in the internal RAMSNAP format (see RamsnapIO for file specification).
 */
class RamsnapWriter : public SnapshotWriter {
private:
    static void storeMagicAndVersion(std::ostream &out);
    static void storeAuxInfo(std::ostream &out, const std::map<std::string, std::string> &auxInfo);
    static void storeBox(std::ostream &out, const TriclinicBox &box);
    static void storeShapes(std::ostream &out, const Packing &packing, const ShapeDataManager &manager);
    static void storeShapeData(std::ostream &out, const ShapeData &data, const ShapeDataManager &manager);

public:
    void write(std::ostream &out, const Packing &packing, const ShapeTraits &traits,
               const std::map<std::string, std::string> &auxInfo) const override
    {
        this->write(out, packing, auxInfo, traits.getDataManager());
    }

    /**
     * @brief Convenient wrapper over
     * write(std::ostream&, const Packing&, const ShapeTraits&, const std::map<std::string, std::string>&) const
     * function only with ShapeDataManager part of ShapeTraits on the argument list.
     */
    void write(std::ostream &out, const Packing &packing, const std::map<std::string, std::string> &auxInfo,
               const ShapeDataManager &manager) const;
};


#endif //RAMPACK_RAMSNAPWRITER_H
