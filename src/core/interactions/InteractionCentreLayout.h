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
 * @brief A helper class storing interaction-centre positions together with their corresponding centre types.
 * @details The class describes the layout of interaction centres within a shape in its reference position and
 * orientation. Centre types may be sparse; consequently, the number of centre types is equal to the largest type id
 * plus one. The canonical spherical case is represented by an empty centres list together with
 * `centreIdxTypeIdxMap == {0}`.
 */
class InteractionCentreLayout {
private:
    std::vector<Vector<3>> centres;
    std::vector<std::size_t> centreIdxTypeIdxMap;
    std::size_t numCentreTypes_{};

    [[nodiscard]] static std::size_t calculateNumCentreTypes(const std::vector<std::size_t> &centreIdxTypeIdxMap) {
        if (centreIdxTypeIdxMap.empty())
            return 0;

        std::size_t maxTypeIdx = 0;
        for (std::size_t typeIdx : centreIdxTypeIdxMap)
            maxTypeIdx = std::max(maxTypeIdx, typeIdx);
        return maxTypeIdx + 1;
    }

public:
    /**
     * @brief Constructs the canonical spherical layout.
     * @details For compatibility with the Interaction API, the spherical layout has an empty list of interaction
     * centres (see Interaction::getInteractionCentres), but the centre-type map respects the required support of
     * the implicit centre idx `0` (see Interaction::calculateEnergyBetween) and maps it to the implicit centre type
     * `0`.
     */
    InteractionCentreLayout() : centreIdxTypeIdxMap{0}, numCentreTypes_{1}
    { }

    /**
     * @brief Constructs a general interaction-centre layout.
     * @param centres interaction-centre positions in the shape reference frame
     * @param centreIdxTypeIdxMap map assigning a centre-type index to each entry of @a centres
     * @details When @a centres is empty, @a centreIdxTypeIdxMap must be equal to `{0}` and the created object
     * represents the canonical spherical layout (see InteractionCentreLayout::InteractionCentreLayout).
     */
    InteractionCentreLayout(std::vector<Vector<3>> centres, std::vector<std::size_t> centreIdxTypeIdxMap)
            : centres{std::move(centres)}, centreIdxTypeIdxMap{std::move(centreIdxTypeIdxMap)},
              numCentreTypes_{calculateNumCentreTypes(this->centreIdxTypeIdxMap)}
    {
        if (this->centres.empty())
            Expects(this->centreIdxTypeIdxMap == std::vector<std::size_t>{0});
        else
            Expects(this->centres.size() == this->centreIdxTypeIdxMap.size());
    }

    /**
     * @brief Returns interaction-centre positions in the shape reference frame
     * (see Interaction::getInteractionCentres).
     * @details An empty vector denotes the canonical spherical layout, i.e., a single interaction centre in the origin.
     */
    [[nodiscard]] const std::vector<Vector<3>> &getCentres() const {
        return this->centres;
    }

    /**
     * @brief Returns the map from interaction-centre index to interaction-centre type.
     * @details For the canonical spherical layout, the returned vector is `{0}` (see
     * InteractionCentreLayout::InteractionCentreLayout).
     */
    [[nodiscard]] const std::vector<std::size_t> &getCentreIdxTypeIdxMap() const {
        return this->centreIdxTypeIdxMap;
    }

    /**
     * @brief Returns the number of interaction centres.
     */
    [[nodiscard]] std::size_t numCentres() const {
        return this->centres.size();
    }

    /**
     * @brief Returns the number of interaction centre types present in the layout.
     * @details The value is equal to the largest centre-type index plus one, therefore sparse centre-type indices are
     * accounted for.
     */
    [[nodiscard]] std::size_t numCentreTypes() const {
        return this->numCentreTypes_;
    }

    friend bool operator==(const InteractionCentreLayout &lhs, const InteractionCentreLayout &rhs) {
        return lhs.centres == rhs.centres && lhs.centreIdxTypeIdxMap == rhs.centreIdxTypeIdxMap;
    }

    friend bool operator!=(const InteractionCentreLayout &lhs, const InteractionCentreLayout &rhs) {
        return !(lhs == rhs);
    }
};


#endif //RAMPACK_INTERACTIONCENTRELAYOUT_H
