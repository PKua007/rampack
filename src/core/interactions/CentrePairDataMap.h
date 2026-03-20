//
// Created by Codex on 16/03/2026.
//

#ifndef RAMPACK_CENTREPAIRDATAMAP_H
#define RAMPACK_CENTREPAIRDATAMAP_H

#include <cmath>
#include <cstddef>
#include <vector>

#include "utils/Exceptions.h"

template <typename PairData>
class CentrePairDataMap {
private:
    std::vector<PairData> pairDataMatrix;

    [[nodiscard]] std::size_t numCentres() const
    {
        return static_cast<std::size_t>(std::sqrt(this->pairDataMatrix.size()));
    }

    [[nodiscard]] std::size_t flatIndex(const std::size_t idx1, const std::size_t idx2) const
    {
        return idx1 * this->numCentres() + idx2;
    }

public:
    explicit CentrePairDataMap(const std::size_t numCentres)
            : pairDataMatrix(numCentres * numCentres, PairData{})
    { }

    void setPairData(const std::size_t idx1, const std::size_t idx2, const PairData &data)
    {
        Expects(idx1 < this->numCentres());
        Expects(idx2 < this->numCentres());

        this->pairDataMatrix[this->flatIndex(idx1, idx2)] = data;
        this->pairDataMatrix[this->flatIndex(idx2, idx1)] = data;
    }

    [[nodiscard]] const PairData &getPairData(const std::size_t idx1, const std::size_t idx2) const
    {
        return this->pairDataMatrix[this->flatIndex(idx1, idx2)];
    }
};


#endif //RAMPACK_CENTREPAIRDATAMAP_H
