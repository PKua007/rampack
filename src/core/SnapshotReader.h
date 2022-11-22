//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_SNAPSHOTREADER_H
#define RAMPACK_SNAPSHOTREADER_H

#include <map>
#include <istream>

#include "Packing.h"
#include "ShapeTraits.h"


/**
 * @brief Class reading system snapshot and printing it onto the packing from an implementation specific source.
 */
class SnapshotReader {
public:
    virtual ~SnapshotReader() = default;

    /**
     * @brief Reads the packing from an implementation specific source and prints is onto the packing.
     * @param in the source to load the snapshot
     * @param packing packing to print the snapshot onto
     * @param traits ShapeTraits of shapes in the packing
     * @return key->value map of auxiliary information if present in the source
     */
    virtual std::map<std::string, std::string> read(std::istream &in, Packing &packing,
                                                    const ShapeTraits &traits) const = 0;
};


#endif //RAMPACK_SNAPSHOTREADER_H
