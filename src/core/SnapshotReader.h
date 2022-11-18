//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_SNAPSHOTREADER_H
#define RAMPACK_SNAPSHOTREADER_H

#include <map>
#include <istream>

#include "Packing.h"
#include "ShapeTraits.h"


class SnapshotReader {
public:
    virtual ~SnapshotReader() = default;

    virtual std::map<std::string, std::string> read(std::istream &in, Packing &packing,
                                                    const ShapeTraits &traits) const = 0;
};


#endif //RAMPACK_SNAPSHOTREADER_H
