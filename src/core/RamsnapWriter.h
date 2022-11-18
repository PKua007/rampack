//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_RAMSNAPWRITER_H
#define RAMPACK_RAMSNAPWRITER_H

#include "SnapshotWriter.h"


class RamsnapWriter : public SnapshotWriter {
public:
    void write(std::ostream &out, const Packing &packing, const ShapeTraits &traits,
               const std::map<std::string, std::string> &auxInfo) const override;
};


#endif //RAMPACK_RAMSNAPWRITER_H
