//
// Created by pkua on 04.04.2022.
//

#ifndef RAMPACK_SIMULATIONIO_H
#define RAMPACK_SIMULATIONIO_H

#include <istream>

#include "TriclinicBox.h"
#include "Shape.h"


class SimulationIO {
protected:
    struct Header {
        char magic[7] = {'R', 'A', 'M', 'T', 'R', 'J', '\n'};
        unsigned char versionMajor = 1;
        unsigned char versionMinor = 0;
        std::size_t numParticles{};
        std::size_t numSnapshots{};
        std::size_t cycleStep{};
    };

    static Header readHeader(std::istream &in);
    static void writeHeader(const Header &header, std::ostream &out);
    static TriclinicBox readBox(std::istream &in);
    static void writeBox(const TriclinicBox &box, std::ostream &out);
    static Shape readShape(std::istream &in);
    static void writeShape(const Shape &shape, std::ostream &out);

    static std::streamoff streamoffForSnapshot(const Header &header, std::size_t snapshotNum);
    static std::size_t getHeaderSize();
    static std::size_t getSnapshotSize(const SimulationIO::Header &header);
};


#endif //RAMPACK_SIMULATIONIO_H
