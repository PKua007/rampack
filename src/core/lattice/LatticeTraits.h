//
// Created by pkua on 19.05.22.
//

#ifndef RAMPACK_LATTICETRAITS_H
#define RAMPACK_LATTICETRAITS_H

#include <array>
#include <string>
#include <stdexcept>

#include "utils/Assertions.h"
#include "UnitCell.h"


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

    using LayerIndices = std::vector<std::size_t>;
    using LayerAssociation = std::vector<std::pair<double, LayerIndices>>;
    using ColumnCoord = std::array<double, 2>;
    using ColumnIndices = std::vector<std::size_t>;
    using ColumnAssociation = std::vector<std::pair<ColumnCoord, ColumnIndices>>;

    /**
     * @brief Converts a string of length 3 with names of axes to their 0-2 indices.
     * @details Namely, for example "zxy" will be converted to an array with elements {2, 0, 1}. Incorrect string
     * throw an exception.
     * @throws AxisOrderParseException if @a axisOrderString is invalid.
     */
    static std::array<std::size_t, 3> parseAxisOrder(const std::string &axisOrderString);

    /**
     * @brief Converts given @a axis to its 0-2 index.
     * @details For example, Axis::Y will be converted to 1.
     */
    static std::size_t axisToIndex(Axis axis);

    static LayerAssociation getLayerAssociation(const UnitCell &cell, Axis layerAxis);
    static ColumnAssociation getColumnAssociation(const UnitCell &cell, Axis columnAxis);
};


#endif //RAMPACK_LATTICETRAITS_H
