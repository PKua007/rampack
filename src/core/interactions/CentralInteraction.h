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
 * @details <p> The class stores pair data for interaction-centre types in CentrePairDataMap and combines it with the
 * currently bound InteractionCentreLayout.
 *
 * <p> Assuming the deriving class is called @a MyInteraction it should derive from CentralInteraction like this
 * (CRTP idiom):
 * @code
 * class MyInteraction : public CentralInteraction<MyInteraction, MyPairData> {
 *     ...
 * }
 * @endcode
 *
 * @tparam Derived concrete interaction type implementing the energy law for a given pair data. Concrete potentials are
 * programmed by implementing methods with signatures
 * @code
 * // Returns interaction energy for a pair of interaction centres separated by distance2 and having pair data pairData.
 * double Derived::calculateEnergyForDistance2(double distance2, const PairData &pairData) const
 *
 * // Returns cut-off distance for a pair of interaction-centre types described by pairData.
 * static double Derived::getRangeRadiusForPairData(const PairData &pairData)
 * @endcode
 * @tparam PairData type storing parameters associated with a pair of interaction centre types
 */
template <typename Derived, typename PairData>
class CentralInteraction : public CentralInteractionBase {
private:
    CentrePairDataMap<PairData> pairDataMap;
    double rangeRadius{};

    [[nodiscard]] static CentrePairDataMap<PairData>
    broadcastUniformPairData(const CentrePairDataMap<PairData> &pairDataMap, std::size_t numCentreTypes)
    {
        Expects(pairDataMap.getNumCentreTypes() == 1);
        CentrePairDataMap<PairData> broadcastPairDataMap(numCentreTypes);
        const PairData &pairData = pairDataMap.getPairData(0, 0);
        for (std::size_t i = 0; i < numCentreTypes; i++)
            for (std::size_t j = i; j < numCentreTypes; j++)
                broadcastPairDataMap.setPairData(i, j, pairData);

        return broadcastPairDataMap;
    }

    [[nodiscard]] double calculateRangeRadius() const {
        double rangeRadius = 0;
        for (std::size_t i = 0; i < this->pairDataMap.getNumCentreTypes(); i++) {
            for (std::size_t j = i; j < this->pairDataMap.getNumCentreTypes(); j++) {
                rangeRadius = std::max(rangeRadius,
                                       Derived::getRangeRadiusForPairData(this->pairDataMap.getPairData(i, j)));
            }
        }
        return rangeRadius;
    }

protected:
    /**
     * @brief Returns pair data for a given pair of interaction-centre types.
     */
    [[nodiscard]] const PairData &getPairData(const std::size_t typeIdx1, const std::size_t typeIdx2) const {
        return this->pairDataMap.getPairData(typeIdx1, typeIdx2);
    }

public:
    /**
     * @brief Constructs the interaction on sphere with the default-constructed PairData.
     * @details The created object is ready to be used with the canonical spherical layout.
     */
    CentralInteraction()
            : CentralInteraction(PairData{})
    { }

    /**
     * @brief Constructs the interaction on sphere.
     * @param pairData pair data for the only pair of interaction centre types present in the spherical case
     * @details The created object stores pair data for a single interaction-centre type and is ready to be used with
     * the canonical spherical layout.
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
     * @details The map may contain more interaction-centre types than required by a layout bound later using
     * bindCentreLayout().
     */
    explicit CentralInteraction(const CentrePairDataMap<PairData> &pairDataMap)
            : pairDataMap(pairDataMap)
    {
        this->rangeRadius = this->calculateRangeRadius();
    }

    /**
     * @brief Binds the interaction to the specified interaction centre layout preserving currently configured pair
     * data.
     * @param interactionCentreLayout interaction-centre positions and their types
     * @param allowUniformPairDataBroadcast if @a true and the interaction currently stores pair data for a single
     * centre type, this singular pair data is copied to all centre-type pairs required by
     * @a interactionCentreLayout
     * @details If uniform broadcast is disabled, or if the interaction already stores pair data for more than one
     * centre type, the number of centre types required by @a interactionCentreLayout must not exceed the number of
     * centre types stored in the pair-data map.
     */
    void bindCentreLayout(const InteractionCentreLayout &interactionCentreLayout,
                          bool allowUniformPairDataBroadcast = false) final
    {
        if (allowUniformPairDataBroadcast && this->pairDataMap.getNumCentreTypes() == 1)
            this->pairDataMap = broadcastUniformPairData(this->pairDataMap, interactionCentreLayout.numCentreTypes());
        else
            Expects(interactionCentreLayout.numCentreTypes() <= this->pairDataMap.getNumCentreTypes());

        this->interactionCentreLayout = interactionCentreLayout;
    }

    [[nodiscard]] double getRangeRadius() const final {
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
        const std::size_t typeIdx1 = this->interactionCentreLayout.getCentreIdxTypeIdxMap()[idx1];
        const std::size_t typeIdx2 = this->interactionCentreLayout.getCentreIdxTypeIdxMap()[idx2];
        const auto &pairData = this->pairDataMap.getPairData(typeIdx1, typeIdx2);

        return static_cast<const Derived *>(this)->calculateEnergyForDistance2(bc.getDistance2(pos1, pos2), pairData);
    }
};


#endif //RAMPACK_CENTRALINTERACTION_H
