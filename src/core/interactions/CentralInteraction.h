//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_CENTRALINTERACTION_H
#define RAMPACK_CENTRALINTERACTION_H

#include <utility>
#include <vector>

#include "CentrePairDataMap.h"
#include "core/Interaction.h"

/**
 * @brief A class representing the central interaction, where the energy depends only on the distance between
 * interaction centres.
 * @details Concrete potentials are programmed by implementing CentralInteraction::calculateEnergyForDistance2 method.
 */
template <typename PairData>
class CentralInteraction : public Interaction {
private:
    std::vector<Vector<3>> potentialCentres;
    std::vector<std::size_t> centreIdxTypeMap;
    CentrePairDataMap<PairData> pairDataMap;

protected:
    /**
     * @brief Method which should be implemented for a concrete central interaction.
     * @param distance2 distance squared between interaction centres
     * @param pairData centre pair data for the concrete pair of interaction centres
     */
    [[nodiscard]] virtual double calculateEnergyForDistance2(double distance2, const PairData& pairData) const = 0;

public:
    /**
     * @brief Constructs the interaction with a single interaction centre in the origin (an empty interaction centres
     * list)
     */
    CentralInteraction() = default;

    /**
     * @brief Constructs the interaction with a concrete list of interaction centres @a potentialCentres.
     */
    explicit CentralInteraction(std::vector<Vector<3>> potentialCentres)
            : potentialCentres{std::move(potentialCentres)}, pairDataMap(1)
    { }

    /**
     * @brief Installs the interaction on sphere (empties the list of interaction centres).
     */
    void installOnSphere(const PairData &pairData) {
        this->potentialCentres = {};
        this->pairDataMap = CentrePairDataMap<PairData>(1);
        this->pairDataMap.setPairData(0, 0, pairData);
    };

    /**
     * @brief Install the interaction on concrete interaction centres.
     */
    void installOnCentres(const std::vector<Vector<3>> &centres, const CentrePairDataMap<PairData> &pairDataMap) {
        this->potentialCentres = centres;
        this->pairDataMap = pairDataMap;
    }

    [[nodiscard]] bool hasHardPart() const final { return false; }
    [[nodiscard]] bool hasWallPart() const final { return false; }
    [[nodiscard]] bool hasSoftPart() const final { return true; }
    [[nodiscard]] bool isConvex() const final { return false; }

    [[nodiscard]] double calculateEnergyBetween(const Vector<3> &pos1,
                                                [[maybe_unused]] const Matrix<3, 3> &orientation1,
                                                const std::size_t idx1,
                                                const Vector<3> &pos2,
                                                [[maybe_unused]] const Matrix<3, 3> &orientation2,
                                                const std::size_t idx2,
                                                const BoundaryConditions &bc) const final
    {
        const std::size_t centreType1 = this->centreIdxTypeMap[idx1];
        const std::size_t centreType2 = this->centreIdxTypeMap[idx2];
        const auto &pairData = this->pairDataMap.getPairData(centreType1, centreType2);

        return this->calculateEnergyForDistance2(bc.getDistance2(pos1, pos2), pairData);
    }

    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const final { return this->potentialCentres; }
};


#endif //RAMPACK_CENTRALINTERACTION_H
