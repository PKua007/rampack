//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_SNAPSHOTWRITER_H
#define RAMPACK_SNAPSHOTWRITER_H

#include <map>
#include <ostream>

#include "Packing.h"
#include "ShapeTraits.h"


class SnapshotWriter {
public:
    virtual ~SnapshotWriter() = default;

    virtual void write(std::ostream &out, const Packing &packing, const ShapeTraits &traits,
                       const std::map<std::string, std::string> &auxInfo) const = 0;
};


#endif //RAMPACK_SNAPSHOTWRITER_H
