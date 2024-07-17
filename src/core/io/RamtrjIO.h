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
 * @brief Base class for storing and restoring simulation trajectories (in a propertiary RAMTRJ format).
 * @details RAMTRJ is a binary format, which stores a simulation trajectory (a series of snapshots) in a compact
 * form, with a random access to particular shapshots. It has the following structure:
 * - **HEADER**:
 *   - `char[7]`: `RAMTRJ\n` magic bytes
 *   - `unsigned char`: version major
 *   - `unsigned char`: version minor
 *   - `unsigned long` (*np*): number of particles
 *   - `unsigned long` (*ns*): number of snapshots
 *   - `unsigned long` (*cs*): number of cycles between snapshots
 * - **SHAPE DATA**:
 *   - `unsigned long` (*nsd*): number of serialized key=value shape parameters as per ShapeDataManager::serialize
 *   - *nsd* &times; `char[?]`: space-delimited list of shape parameter keys
 *   - *np* &times; *nsd* &times; `char[?]`: list of shape parameter values, first grouped in *nsd* values
 *     corresponding to the keys in the order specified above for a single shape
 * - **SNAPSHOT DATA**: *ns* snapshots, each in the format:
 *   - 9 &times; `double`: TriclinicBox matrix, stored row-wise
 *   - *np* particles, each in the format:
 *     - 3 &times; `double`: absolute position of the shape
 *     - 3 &times; `double`: Euler angles of the shape (angle of rotations around x, y, and z axis, performed in this
 *       order, in radians)
 *
 * Space-delimited entries `char[?]` are strings consisting of non-whitespace characters of an unknown length, which are
 * read byte by byte until whitespace is encountered (it is read but not included). ShapeData are assumed to be constant
 * between the snapshots, thus the list of them for each particle is stored only once, after the header.
 *
 * Version history:
 * - **1.0**:
 *   - first release
 * - **1.1**:
 *   - *np* and *cs* in the **HEADER** section are set before recording snapshots and they can no longer be zero
 * - **1.2**:
 *   - addition of **SHAPE DATA** section
 */
class RamtrjIO {
public:
    /** @brief Current version of RAMTRJ API. */
    static constexpr Version CURRENT_VERSION = {1, 2};

    /** @brief Version since when Header::numParticles and Header::cycleStep cannot be zero before first snapshot is
     * registered. */
    static constexpr Version NONZERO_DATA_VERSION = {1, 1};

    /** @brief Version since shape data were introduced. */
    static constexpr Version SHAPE_DATA_VERSION = {1, 2};

private:
    static std::string readWhitespaceDelimitedString(std::istream &in);
    static void writeSpaceDelimitedString(const std::string &str, std::ostream &out);

protected:
    /**
     * @brief Header of the RAMTRJ file with basic metadata and ShapeData of all particles.
     */
    struct Header {
        /** @brief Invalid (unset) offset of shape data. */
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
     * @brief Reads the header and the shape data in a binary format from @a in input stream.
     */
    static Header readHeaderAndShapeData(const ShapeDataManager &manager, std::istream &in);

    /**
     * @brief Writes the header and the shape data in a binary format to @a out output stream.
     * @details Header::firstShapshotOffset is updated to a proper offset int @a out.
     */
    static void writeHeaderAndShapeData(Header &header, const ShapeDataManager &manager, std::ostream &out);

    /**
     * @brief Writes the header alone (without the shape data) in a binary format from @a in input stream.
     */
    static void writeHeader(const Header &header, std::ostream &out);

    /**
     * @brief Reads the shape data in a binary format from @a in input stream.
     */
    static std::vector<ShapeData> readShapeData(const ShapeDataManager &manager, std::size_t numParticles,
                                                std::istream &in);

    /**
     * @brief Reads the shape data in a binary format from @a in input stream.
     */
    static void writeShapeData(const std::vector<ShapeData> &shapeDatas, const ShapeDataManager &manager,
                               std::ostream &out);

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
