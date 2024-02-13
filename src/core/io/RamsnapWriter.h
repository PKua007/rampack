//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_RAMSNAPWRITER_H
#define RAMPACK_RAMSNAPWRITER_H

#include "core/SnapshotWriter.h"


/**
 * @brief Stores the snapshot in the internal RAMSNAP format.
 * @details <p> The format is human readable with a structure:
 * <ol>
 * <li>number of auxiliary fields
 * <li>newline separated pairs `key value` with auxiliary info
 * <li>space separated 9 entries of TriclinicBox matrix (see TriclinicBox::TriclinicBox(const Matrix<3, 3> &)). The
 *     matrix is stores row-wise
 * <li>number of shapes in the packing
 * <li>rows with shapes - space separated 12 values. First 3 are position, next 9 are row-wise stored rotation matrix
 * </ol>
 *
 * <p> Example file
 * @code
 * 3
 * cycles 1000000
 * step.scaling 0.1
 * step.translation.translation 0.01
 * 10 0 0  0 10 0  0 0 10
 * 2
 * 2 2 2   1 0 0  0 1 0  0 0 1
 * 8 8 8   0 1 0  -1 0 0  0 0 1
 * @endcode
 */
class RamsnapWriter : public SnapshotWriter {
private:
    static void storeAuxInfo(std::ostream &out, const std::map<std::string, std::string> &auxInfo);
    static void storeBox(std::ostream &out, const TriclinicBox &box);
    static void storeShapes(std::ostream &out, const Packing &packing, const ShapeDataManager &manager);
    static void storeShapeData(std::ostream &out, const ShapeData &data, const ShapeDataManager &manager);
    static std::string quoteIfNecessary(const std::string &str);

public:
    void write(std::ostream &out, const Packing &packing, [[maybe_unused]] const ShapeTraits &traits,
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
