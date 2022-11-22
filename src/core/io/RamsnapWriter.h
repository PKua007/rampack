//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_RAMSNAPWRITER_H
#define RAMPACK_RAMSNAPWRITER_H

#include "core/SnapshotWriter.h"


class RamsnapWriter : public SnapshotWriter {
private:
    static void storeAuxInfo(std::ostream &out, const std::map<std::string, std::string> &auxInfo);
    static void storeBox(std::ostream &out, const TriclinicBox &box);
    static void storeShapes(std::ostream &out, const Packing &packing);

public:
    void write(std::ostream &out, const Packing &packing, [[maybe_unused]] const ShapeTraits &traits,
               const std::map<std::string, std::string> &auxInfo) const override
    {
        this->write(out, packing, auxInfo);
    }

    void write(std::ostream &out, const Packing &packing, const std::map<std::string, std::string> &auxInfo) const;
};


#endif //RAMPACK_RAMSNAPWRITER_H
