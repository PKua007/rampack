//
// Created by pkua on 18.11.22.
//

#ifndef RAMPACK_XYZWRITER_H
#define RAMPACK_XYZWRITER_H

#include "core/SnapshotWriter.h"


class XYZWriter : public SnapshotWriter {
private:
    static void storeHeader(std::ostream &out, const Packing &packing,
                            const std::map<std::string, std::string> &auxInfo);
    static void storeShapes(std::ostream &out, const Packing &packing);
    static void storeAuxInfo(std::ostream &out, const std::map<std::string, std::string> &auxInfo);

public:
    void write(std::ostream &out, const Packing &packing, [[maybe_unused]] const ShapeTraits &traits,
               const std::map<std::string, std::string> &auxInfo) const override
    {
        this->write(out, packing, auxInfo);
    }

    void write(std::ostream &out, const Packing &packing, const std::map<std::string, std::string> &auxInfo) const;
};


#endif //RAMPACK_XYZWRITER_H
