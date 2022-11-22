//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_XYZWRITER_H
#define RAMPACK_XYZWRITER_H

#include "core/SnapshotWriter.h"


class XYZWriter : public SnapshotWriter {
public:
    void write(std::ostream &out, const Packing &packing, const ShapeTraits &traits,
               const std::map<std::string, std::string> &auxInfo) const override;
};


#endif //RAMPACK_XYZWRITER_H
