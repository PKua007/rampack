//
// Created by pkua on 19.05.22.
//

#ifndef RAMPACK_LATTICETRAITS_H
#define RAMPACK_LATTICETRAITS_H

#include <array>
#include <string>

#include "utils/Assertions.h"


class LatticeTraits {
public:
    /**
     * @brief Enumeration of coordinate system axes.
     */
    enum class Axis {
        X,
        Y,
        Z
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

    static std::array<std::size_t, 3> parseAxisOrder(const std::string &axisOrderString);
    static std::size_t axisToIndex(Axis axis);
};


#endif //RAMPACK_LATTICETRAITS_H
