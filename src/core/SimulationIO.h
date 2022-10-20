//
// Created by pkua on 04.04.2022.
//

#ifndef RAMPACK_SIMULATIONIO_H
#define RAMPACK_SIMULATIONIO_H

#include <istream>

#include "TriclinicBox.h"
#include "Shape.h"


/**
 * @brief Base class for storing and restoring simulation trajectories (in a propertiary RAMTRJ format)
 */
class SimulationIO {
protected:
    /**
     * @brief Header of RAMTRJ file (as is)
     * @details Version log
     * <ol>
     * <li> 1.0 - first release
     * <li> 1.1 - @a numParticles and @a cycleStep set before recording snapshots (header no longer has zeros)
     * </ol>
     */
    struct Header {
        char magic[7] = {'R', 'A', 'M', 'T', 'R', 'J', '\n'};
        unsigned char versionMajor = 1;
        unsigned char versionMinor = 1;
        std::size_t numParticles{};
        std::size_t numSnapshots{};
        std::size_t cycleStep{};
    };

    /**
     * @brief Reads the header in a binary format from @a in input stream.
     */
    static Header readHeader(std::istream &in);

    /**
     * @brief Writes the header in a binary format to @a out output stream.
     */
    static void writeHeader(const Header &header, std::ostream &out);

    /**
     * @brief Reads the simulation box dimensions in a binary format from @a in input stream.
     */
    static TriclinicBox readBox(std::istream &in);

    /**
     * @brief Writes the simulation box dimensions in a binary format to @a out output stream.
     */
    static void writeBox(const TriclinicBox &box, std::ostream &out);

    /**
     * @brief Reads the shape (position and orientation) in a binary format from @a in input stream.
     */
    static Shape readShape(std::istream &in);

    /**
     * @brief Writes the shape (position and orientation) in a binary format to @a out output stream.
     */
    static void writeShape(const Shape &shape, std::ostream &out);

    /**
     * @brief Returns @a seekp/tellp byte offset of a beginning of a @a snapshotNum snapshot
     */
    static std::streamoff streamoffForSnapshot(const Header &header, std::size_t snapshotNum);

    /**
     * @brief Returns the size of header in bytes as stored in the file (different to @a sizeof(Header) due to padding!)
     */
    static std::size_t getHeaderSize();

    /**
     * @brief Returns the size of a single snapshot as storef in the file.
     */
    static std::size_t getSnapshotSize(const SimulationIO::Header &header);
};


#endif //RAMPACK_SIMULATIONIO_H
