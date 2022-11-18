//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_RAMSNAPREADER_H
#define RAMPACK_RAMSNAPREADER_H

#include "SnapshotReader.h"


class RamsnapReader : public SnapshotReader {
private:
    static std::map<std::string, std::string> restoreAuxInfo(std::istream &in);
    static Matrix<3, 3> restoreDimensions(std::istream &in);

public:
    std::map<std::string, std::string> read(std::istream &in, Packing &packing,
                                            const ShapeTraits &traits) const override;
};


#endif //RAMPACK_RAMSNAPREADER_H
