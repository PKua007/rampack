//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_SNAPSHOTWRITER_H
#define RAMPACK_SNAPSHOTWRITER_H

#include <map>
#include <ostream>

#include "Packing.h"
#include "ShapeTraits.h"


/**
 * @brief Class storing a single system snapshot in a specific format.
 */
class SnapshotWriter {
public:
    virtual ~SnapshotWriter() = default;

    /**
     * @brief Stores current state of the packing.
     * @details Whether all information is stored is implementation specific. For example, some implementations may
     * completely ignore @a auxInfo.
     * @param out output stream to store the packing
     * @param packing packing to be stored
     * @param traits ShapeTraits of the shapes in the packing
     * @param auxInfo additional, auxiliary key->value pairs to store
     */
    virtual void write(std::ostream &out, const Packing &packing, const ShapeTraits &traits,
                       const std::map<std::string, std::string> &auxInfo) const = 0;
};


#endif //RAMPACK_SNAPSHOTWRITER_H
