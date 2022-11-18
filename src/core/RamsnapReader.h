//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_RAMSNAPREADER_H
#define RAMPACK_RAMSNAPREADER_H

#include "SnapshotReader.h"


class RamsnapReader : public SnapshotReader {
private:
    static Matrix<3, 3> restoreDimensions(std::istream &in);

public:
    static std::map<std::string, std::string> restoreAuxInfo(std::istream &in);

    std::map<std::string, std::string> read(std::istream &in, Packing &packing,
                                            const ShapeTraits &traits) const override
    {
        return this->read(in, packing, traits.getInteraction());
    }

    std::map<std::string, std::string> read(std::istream &in, Packing &packing,
                                            const Interaction &interaction) const;
};


#endif //RAMPACK_RAMSNAPREADER_H
