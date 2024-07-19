//
// Created by pkua on 19.05.22.
//

#ifndef RAMPACK_LATTICETRAITS_H
#define RAMPACK_LATTICETRAITS_H

#include <array>
#include <string>
#include <stdexcept>

#include "utils/Exceptions.h"
#include "UnitCell.h"
#include "Lattice.h"
#include "core/Packing.h"


/**
 * @brief Class gathering some lattice characteristics and helper functions.
 */
class LatticeTraits {
public:
    /**
     * @brief Enumeration of coordinate system axes.
     */
    enum class Axis {
        /** @brief X axis */
        X,
        /** @brief Y axis */
        Y,
        /** @brief Z axis */
        Z
    };

    /**
     * @brief Exception thrown by parseAxisOrder() if argument is invalid.
     */
    struct AxisOrderParseException : public std::runtime_error {
        explicit AxisOrderParseException(const std::string &what) : std::runtime_error(what) { }
    };

    /**
     * @brief Enumeration of layer clinicity.
     */
    enum class Clinicity {
        /** @brief Implicit (default) clinicity */
        IMPLICIT,
        /** @brief Synclinic (not-alternating) tilt arrangement */
        SYNCLINIC,
        /** @brief Anticlinic (alternating) tilt arrangement */
        ANTICLINIC
    };

    /**
     * @brief Enumeration of layer polarization.
     */
    enum class Polarization {
        /** @brief Implicit (default) polarization */
        IMPLICIT,
        /** @brief Ferroelectic polar arrangement */
        FERRO,
        /** @brief Antiferroelectic (antipolar) polar arrangement */
        ANTIFERRO
    };

    /**
     * @brief Alias for a list of indices of particles in the UnitCell belonging to a particular layer of
     * @ref LayerAssociation.
     */
    using LayerIndices = std::vector<std::size_t>;

    /**
     * @brief Alias for the description of layers in the UnitCell - an association between layer's relative coordinate
     * (undisclosed along which axis) and its @ref LayerIndices
     */
    using LayerAssociation = std::vector<std::pair<double, LayerIndices>>;

    /**
     * @brief Alias for two relative coordinates (undisclosed along which axes) of a column in @ref ColumnAssociation.
     */
    using ColumnCoord = std::array<double, 2>;

    /**
     * @brief Alias for a list of indices of particles in the UnitCell belonging to a particular column of
     * @ref ColumnAssociation.
     */
    using ColumnIndices = std::vector<std::size_t>;

    /**
     * @brief Alias for the description of columns in the UnitCell - an association between column's @ref ColumnCoord
     * and its @ref ColumnIndices.
     */
    using ColumnAssociation = std::vector<std::pair<ColumnCoord, ColumnIndices>>;

    /**
     * @brief Converts a string of length 3 with names of axes to their 0-2 indices.
     * @details Namely, for example "zxy" will be converted to an array with elements {2, 0, 1}. Invalid string throws
     * an exception.
     * @throws AxisOrderParseException if @a axisOrderString is invalid.
     */
    static std::array<std::size_t, 3> parseAxisOrder(const std::string &axisOrderString);

    /**
     * @brief Converts given @a axis to its 0-2 index.
     * @details For example, Axis::Y will be converted to 1.
     */
    static std::size_t axisToIndex(Axis axis);

    /**
     * @brief Computes @ref LayerAssociation for a given @a cell. A single layer consist of particles, whose coordinate
     * along @a layerAxis is the same.
     */
    static LayerAssociation getLayerAssociation(const UnitCell &cell, Axis layerAxis);

    /**
     * @brief Computes @ref ColumnAssociation for a given @a cell. A single column consist of particles, whose two
     * coordinates which are not along @a columnAxis are the same.
     */
    static ColumnAssociation getColumnAssociation(const UnitCell &cell, Axis columnAxis);

    /**
     * @brief Creates a Lattice with a single unit cell based on @a packing, copying its box shape and particles.
     */
    static Lattice latticeFromPacking(const Packing &packing);
};


#endif //RAMPACK_LATTICETRAITS_H
