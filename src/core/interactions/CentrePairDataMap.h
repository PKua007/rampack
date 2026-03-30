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
 * is symmetric with respect to the order of the centre types, therefore data for a pair `(i, j)` is identical to the
 * data for `(j, i)`.
 */
template <typename PairData>
class CentrePairDataMap {
private:
    std::size_t numCentreTypes{};
    std::vector<PairData> pairDataMatrix;

    [[nodiscard]] std::size_t flatIndex(const std::size_t typeIdx1, const std::size_t typeIdx2) const {
        return typeIdx1 * this->numCentreTypes + typeIdx2;
    }

public:
    /**
     * @brief Constructs an empty map with no interaction centre types.
     */
    CentrePairDataMap() = default;

    /**
     * @brief Constructs the map for @a numCentreTypes interaction centre types.
     * @details All pair data entries are value-initialized.
     */
    explicit CentrePairDataMap(const std::size_t numCentreTypes)
            : numCentreTypes{numCentreTypes}, pairDataMatrix(numCentreTypes * numCentreTypes, PairData{})
    { }

    /**
     * @brief Returns the number of interaction centre types stored in the map.
     */
    [[nodiscard]] std::size_t getNumCentreTypes() const
    {
        return this->numCentreTypes;
    }

    /**
     * @brief Sets pair data for interaction centre types @a typeIdx1 and @a typeIdx2.
     * @details Since the map stores symmetric pair data, both `(typeIdx1, typeIdx2)` and
     * `(typeIdx2, typeIdx1)` entries are updated.
     */
    void setPairData(const std::size_t typeIdx1, const std::size_t typeIdx2, const PairData &data) {
        Expects(typeIdx1 < this->numCentreTypes);
        Expects(typeIdx2 < this->numCentreTypes);

        this->pairDataMatrix[this->flatIndex(typeIdx1, typeIdx2)] = data;
        this->pairDataMatrix[this->flatIndex(typeIdx2, typeIdx1)] = data;
    }

    /**
     * @brief Returns pair data for interaction centre types @a typeIdx1 and @a typeIdx2.
     */
    [[nodiscard]] const PairData &getPairData(const std::size_t typeIdx1, const std::size_t typeIdx2) const {
        return this->pairDataMatrix[this->flatIndex(typeIdx1, typeIdx2)];
    }
};


#endif //RAMPACK_CENTREPAIRDATAMAP_H
