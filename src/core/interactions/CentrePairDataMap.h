//
// Created by Codex on 16/03/2026.
//

#ifndef RAMPACK_CENTREPAIRDATAMAP_H
#define RAMPACK_CENTREPAIRDATAMAP_H

#include <cstddef>
#include <vector>

#include "utils/Exceptions.h"

/**
 * @brief A helper class storing PairData for all pairs of interaction centre types.
 * @details The data is stored in a square matrix indexed by interaction centre types. The class assumes the pair data
 * is symmetric with respect to the order of the centre types.
 */
template <typename PairData>
class CentrePairDataMap {
private:
    std::size_t numCentres{};
    std::vector<PairData> pairDataMatrix;

    [[nodiscard]] std::size_t flatIndex(const std::size_t idx1, const std::size_t idx2) const
    {
        return idx1 * this->numCentres + idx2;
    }

public:
    /**
     * @brief Constructs an empty map.
     */
    CentrePairDataMap() = default;

    /**
     * @brief Constructs the map for @a numCentres interaction centre types.
     * @details All pair data entries are value-initialized.
     */
    explicit CentrePairDataMap(const std::size_t numCentres)
            : numCentres{numCentres}, pairDataMatrix(numCentres * numCentres, PairData{})
    { }

    /**
     * @brief Returns the number of interaction centre types stored in the map.
     */
    [[nodiscard]] std::size_t getNumCentres() const
    {
        return this->numCentres;
    }

    /**
     * @brief Sets pair data for interaction centre types @a idx1 and @a idx2.
     * @details Since the map stores symmetric pair data, both `(idx1, idx2)` and `(idx2, idx1)` entries are updated.
     */
    void setPairData(const std::size_t idx1, const std::size_t idx2, const PairData &data)
    {
        Expects(idx1 < this->numCentres);
        Expects(idx2 < this->numCentres);

        this->pairDataMatrix[this->flatIndex(idx1, idx2)] = data;
        this->pairDataMatrix[this->flatIndex(idx2, idx1)] = data;
    }

    /**
     * @brief Returns pair data for interaction centre types @a idx1 and @a idx2.
     */
    [[nodiscard]] const PairData &getPairData(const std::size_t idx1, const std::size_t idx2) const
    {
        return this->pairDataMatrix[this->flatIndex(idx1, idx2)];
    }
};


#endif //RAMPACK_CENTREPAIRDATAMAP_H
