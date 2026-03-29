//
// Created by Codex on 21/03/2026.
//

#ifndef RAMPACK_INTERACTIONCENTRELAYOUT_H
#define RAMPACK_INTERACTIONCENTRELAYOUT_H

#include <algorithm>
#include <cstddef>
#include <vector>

#include "geometry/Vector.h"
#include "utils/Exceptions.h"

/**
 * @brief A helper class storing interaction centre positions and their corresponding centre types.
 */
class InteractionCentreLayout {
private:
    std::vector<Vector<3>> centres;
    std::vector<std::size_t> centreIdxTypeMap;
    std::size_t numCentreTypes_{};

    [[nodiscard]] static std::size_t calculateNumCentreTypes(const std::vector<std::size_t> &centreIdxTypeMap)
    {
        if (centreIdxTypeMap.empty())
            return 0;

        std::size_t maxCentreType = 0;
        for (std::size_t centreType : centreIdxTypeMap)
            maxCentreType = std::max(maxCentreType, centreType);
        return maxCentreType + 1;
    }

public:
    InteractionCentreLayout()
            : centreIdxTypeMap{0}, numCentreTypes_{1}
    { }

    InteractionCentreLayout(std::vector<Vector<3>> centres, std::vector<std::size_t> centreIdxTypeMap)
            : centres{std::move(centres)}, centreIdxTypeMap{std::move(centreIdxTypeMap)},
              numCentreTypes_{calculateNumCentreTypes(this->centreIdxTypeMap)}
    {
        if (this->centres.empty())
            Expects(this->centreIdxTypeMap == std::vector<std::size_t>{0});
        else
            Expects(this->centres.size() == this->centreIdxTypeMap.size());
    }

    [[nodiscard]] const std::vector<Vector<3>> &getCentres() const
    {
        return this->centres;
    }

    [[nodiscard]] const std::vector<std::size_t> &getCentreIdxTypeMap() const
    {
        return this->centreIdxTypeMap;
    }

    /**
     * @brief Returns the number of interaction centres.
     */
    [[nodiscard]] std::size_t numCentres() const
    {
        return this->centres.size();
    }

    /**
     * @brief Returns the number of interaction centre types present in the layout.
     */
    [[nodiscard]] std::size_t numCentreTypes() const
    {
        return this->numCentreTypes_;
    }

    friend bool operator==(const InteractionCentreLayout &lhs, const InteractionCentreLayout &rhs)
    {
        return lhs.centres == rhs.centres && lhs.centreIdxTypeMap == rhs.centreIdxTypeMap;
    }

    friend bool operator!=(const InteractionCentreLayout &lhs, const InteractionCentreLayout &rhs)
    {
        return !(lhs == rhs);
    }
};


#endif //RAMPACK_INTERACTIONCENTRELAYOUT_H
