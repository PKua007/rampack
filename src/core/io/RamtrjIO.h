//
// Created by pkua on 04.04.2022.
//

#ifndef RAMPACK_RAMTRJIO_H
#define RAMPACK_RAMTRJIO_H

#include <istream>

#include "core/TriclinicBox.h"
#include "core/Shape.h"
#include "utils/Exceptions.h"
#include "core/ShapeDataManager.h"
#include "utils/Version.h"


/**
 * @brief Exception thrown when reading/writing RAMTRJ data.
 */
class RamtrjException : public ValidationException {
public:
    using ValidationException::ValidationException;
};

/**
 * @brief Base class for storing and restoring simulation trajectories (in a propertiary RAMTRJ format)
 */
class RamtrjIO {
public:
    static constexpr Version CURRENT_VERSION = {1, 2};

    static constexpr Version NONZERO_DATA_VERSION = {1, 1};
    static constexpr Version SHAPE_DATA_VERSION = {1, 2};

protected:
    /**
     * @brief Header of RAMTRJ file (as is)
     * @details Version log
     * <ol>
     * <li> 1.0 - first release
     * <li> 1.1 - @a numParticles and @a cycleStep set before recording snapshots (header no longer has zeros)
     * <li> 1.2 - support for shape data - it is stored once, after the header
     * </ol>
     */
    struct Header {
        static constexpr std::streamoff INVALID_OFFSET = -1;

        char magic[7] = {'R', 'A', 'M', 'T', 'R', 'J', '\n'};
        unsigned char versionMajor = CURRENT_VERSION.getMajor();
        unsigned char versionMinor = CURRENT_VERSION.getMinor();
        std::size_t numParticles{};
        std::size_t numSnapshots{};
        std::size_t cycleStep{};
        std::streamoff firstSnapshotOffset = INVALID_OFFSET;
        std::vector<ShapeData> shapeDatas;

        [[nodiscard]] Version getVersion() const { return {this->versionMajor, this->versionMinor}; }
    };

    /**
     * @brief Reads the header in a binary format from @a in input stream.
     */
    static Header readHeaderAndShapeData(const ShapeDataManager &manager, std::istream &in);

    /**
     * @brief Writes the header in a binary format to @a out output stream.
     */
    static void writeHeaderAndShapeData(Header &header, const ShapeDataManager &manager, std::ostream &out);

    static void writeHeader(const Header &header, std::ostream &out);

    static std::vector<ShapeData> readShapeData(const ShapeDataManager &manager, std::size_t numParticles,
                                                std::istream &in);

    static void writeShapeData(const std::vector<ShapeData> &shapeDatas, const ShapeDataManager &manager,
                               std::ostream &out);

    static std::string readWhitespaceDelimitedString(std::istream &in);

    static void writeSpaceDelimitedString(const std::string &str, std::ostream &out);

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
    static Shape readShape(const Header &header, std::size_t particleIdx, std::istream &in);

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
    static std::size_t getSnapshotSize(const RamtrjIO::Header &header);

};


#endif //RAMPACK_RAMTRJIO_H
