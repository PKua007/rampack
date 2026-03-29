//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_CENTRALINTERACTION_H
#define RAMPACK_CENTRALINTERACTION_H

#include <algorithm>

#include "CentrePairDataMap.h"
#include "CentralInteractionBase.h"

/**
 * @brief A class representing the central interaction, where the energy depends only on the distance between
 * interaction centres and their type.
 * @tparam PairData type storing parameters associated with a pair of interaction centre types
 * @details The class stores interaction centre types and maps pairs of these types to PairData objects. Concrete
 * potentials are programmed by implementing CentralInteraction::calculateEnergyForDistance2 method, which receives
 * PairData for a given pair of interaction centres and may use it in energy computations.
 */
template <typename Derived, typename PairData>
class CentralInteraction : public CentralInteractionBase {
private:
    CentrePairDataMap<PairData> pairDataMap;
    double rangeRadius{};

    [[nodiscard]] static CentrePairDataMap<PairData> broadcastUniformPairData(
            const CentrePairDataMap<PairData> &pairDataMap, std::size_t numCentreTypes)
    {
        Expects(pairDataMap.getNumCentres() == 1);
        CentrePairDataMap<PairData> broadcastPairDataMap(numCentreTypes);
        const PairData &pairData = pairDataMap.getPairData(0, 0);
        for (std::size_t i = 0; i < numCentreTypes; i++) {
            for (std::size_t j = i; j < numCentreTypes; j++)
                broadcastPairDataMap.setPairData(i, j, pairData);
        }
        return broadcastPairDataMap;
    }

    [[nodiscard]] double calculateRangeRadius() const
    {
        double rangeRadius = 0;
        for (std::size_t i = 0; i < this->pairDataMap.getNumCentres(); i++) {
            for (std::size_t j = i; j < this->pairDataMap.getNumCentres(); j++) {
                rangeRadius = std::max(rangeRadius, Derived::getRangeRadiusForPairData(this->pairDataMap.getPairData(i, j)));
            }
        }
        return rangeRadius;
    }

protected:
    [[nodiscard]] const PairData &getPairData(const std::size_t idx1, const std::size_t idx2) const
    {
        return this->pairDataMap.getPairData(idx1, idx2);
    }

public:
    /**
     * @brief Constructs the interaction on sphere with the default-constructed PairData.
     */
    CentralInteraction()
            : CentralInteraction(PairData{})
    { }

    /**
     * @brief Constructs the interaction on sphere.
     * @param pairData pair data for the only pair of interaction centre types present in the spherical case
     */
    explicit CentralInteraction(const PairData &pairData)
            : pairDataMap(1)
    {
        this->pairDataMap.setPairData(0, 0, pairData);
        this->rangeRadius = this->calculateRangeRadius();
    }

    /**
     * @brief Constructs the interaction with explicit pair data map and the default spherical layout.
     * @param pairDataMap map of pair data for all pairs of interaction centre types
     */
    explicit CentralInteraction(const CentrePairDataMap<PairData> &pairDataMap)
            : pairDataMap(pairDataMap)
    {
        this->rangeRadius = this->calculateRangeRadius();
    }

    /**
     * @brief Binds the interaction to the specified interaction centre layout preserving currently configured pair
     * data.
     */
    void bindCentreLayout(const InteractionCentreLayout &interactionCentreLayout,
                          bool allowUniformPairDataBroadcast = false) final
    {
        if (allowUniformPairDataBroadcast && this->pairDataMap.getNumCentres() == 1)
            this->pairDataMap = broadcastUniformPairData(this->pairDataMap, interactionCentreLayout.numCentreTypes());
        else
            Expects(interactionCentreLayout.numCentreTypes() <= this->pairDataMap.getNumCentres());

        this->interactionCentreLayout = interactionCentreLayout;
    }

    [[nodiscard]] double getRangeRadius() const final
    {
        return this->rangeRadius;
    }

    [[nodiscard]] double calculateEnergyBetween(const Vector<3> &pos1,
                                                [[maybe_unused]] const Matrix<3, 3> &orientation1,
                                                const std::size_t idx1,
                                                const Vector<3> &pos2,
                                                [[maybe_unused]] const Matrix<3, 3> &orientation2,
                                                const std::size_t idx2,
                                                const BoundaryConditions &bc) const final
    {
        const std::size_t centreType1 = this->interactionCentreLayout.getCentreIdxTypeMap()[idx1];
        const std::size_t centreType2 = this->interactionCentreLayout.getCentreIdxTypeMap()[idx2];
        const auto &pairData = this->pairDataMap.getPairData(centreType1, centreType2);

        return static_cast<const Derived *>(this)->calculateEnergyForDistance2(bc.getDistance2(pos1, pos2), pairData);
    }
};


#endif //RAMPACK_CENTRALINTERACTION_H
