//
// Created by Piotr Kubala on 20/02/2024.
//

#ifndef RAMPACK_RAMSNAPIO_H
#define RAMPACK_RAMSNAPIO_H

#include "utils/Version.h"


/**
 * @brief Common constants and helper methods for RAMSNAP classes.
 * @details <p> The format is human readable with the following structure:
 * - `RAMSNAP` magic token followed by version major and version minor (all space separated)
 * - number of auxiliary fields
 * - newline separated `[key] [value]` pairs with auxiliary info
 * - either of:
 *   - space separated 9 entries of TriclinicBox matrix (see TriclinicBox::TriclinicBox(const Matrix<3, 3> &)). The
 *     matrix is stores row-wise
 *   - space separated 3 entries of TriclinicBox side lengths (see
 *     TriclinicBox::TriclinicBox(const std::array<double, 3> &)).
 * - number of shapes in the packing
 * - rows with shapes. These contain, subsequently:
 *   - 3 position coordinates
 *   - 9 entries of rotation matrix stored row-wise
 *   - number of ShapeData parameters
 *   - space separated pairs of `[key] [value]` ShapeDataManager::serialize map entries. Keys are sorted alphabetically
 *
 * <p> Example file (the example contains some additional spaces for readability):
 * @code
 * RAMSNAP 1 1
 * 3
 * cycles 1000000
 * step.scaling 0.1
 * step.translation.translation 0.01
 * 10 0 0  0 10 0  0 0 10
 * 2
 * 2 2 2   1 0 0   0 1 0  0 0 1   2  r 0.5  l 1.5
 * 8 8 8   0 1 0  -1 0 0  0 0 1   2  r 0.7  l 1.3
 * @endcode
 *
 * <p> Version history:
 * - **1.0**:
 *   - first, implicit version
 * - **1.1**:
 *   - added line with magic and version at the beginning
 *   - added serialized ShapeData at the end of each Shape line entry
 */
class RamsnapIO {
public:
    /** @brief Current version of the RAMSNAP format. */
    static constexpr Version CURRENT_VERSION = {1, 1};

    /** @brief The version where ShapeData were first introduced. */
    static constexpr Version SHAPE_DATA_VERSION = {1, 1};
};


#endif //RAMPACK_RAMSNAPIO_H
