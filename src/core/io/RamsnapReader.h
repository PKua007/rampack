//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_RAMSNAPREADER_H
#define RAMPACK_RAMSNAPREADER_H

#include "core/SnapshotReader.h"


class RamsnapReader : public SnapshotReader {
private:
    static TriclinicBox restoreBox(std::istream &in);
    static std::vector<Shape> restoreShapes(std::istream &in);

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
