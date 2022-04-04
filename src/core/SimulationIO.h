//
// Created by pkua on 04.04.2022.
//

#ifndef RAMPACK_SIMULATIONIO_H
#define RAMPACK_SIMULATIONIO_H

#include <istream>

#include "TriclinicBox.h"
#include "Shape.h"


class SimulationIO {
public:
    struct Header {
        char magic[6] = {'R', 'A', 'M', 'T', 'R', 'J'};
        char newline = '\n';
        unsigned char versionMajor = 1;
        unsigned char versionMinor = 0;
        std::size_t numParticles{};
        std::size_t numSnapshots{};
        std::size_t cycleStep{};
    };

    struct BoxData {
        [[nodiscard]] TriclinicBox toTriclinicBox() const;
        void fromTriclinicBox(const TriclinicBox &box);

        double dimensions[9]{};
    };

    struct ParticleData {
        [[nodiscard]] Shape toShape() const;
        void fromShape(const Shape &shape);

        double position[3]{};
        double eulerAngles[3]{};
    };

protected:
    static Header readHeader(std::istream &stream);
    static std::streamoff posForSnapshot(const Header &header, std::size_t snapshotNum);
};


#endif //RAMPACK_SIMULATIONIO_H
