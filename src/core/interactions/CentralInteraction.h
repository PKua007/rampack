//
// Created by Piotr Kubala on 22/12/2020.
//

#ifndef RAMPACK_CENTRALINTERACTION_H
#define RAMPACK_CENTRALINTERACTION_H

#include <algorithm>
#include <utility>
#include <vector>

#include "CentrePairDataMap.h"
#include "core/Interaction.h"

/**
 * @brief A class representing the central interaction, where the energy depends only on the distance between
 * interaction centres and their type.
 * @tparam PairData type storing parameters associated with a pair of interaction centre types
 * @details The class stores interaction centre types and maps pairs of these types to PairData objects. Concrete
 * potentials are programmed by implementing CentralInteraction::calculateEnergyForDistance2 method, which receives
 * PairData for a given pair of interaction centres and may use it in energy computations.
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
     * @brief Constructs the interaction. The default setup is a sphere (installOnSphere()) with the default-constructed
     * PairData.
     */
    CentralInteraction() { this->installOnSphere(PairData{}); }

    /**
     * @brief Installs the interaction on sphere (empties the list of interaction centres).
     * @param pairData pair data for the only pair of interaction centre types present in the spherical case
     */
    void installOnSphere(const PairData &pairData) {
        this->potentialCentres = {};
        this->centreIdxTypeMap = {0};
        this->pairDataMap = CentrePairDataMap<PairData>(1);
        this->pairDataMap.setPairData(0, 0, pairData);
    };

    /**
     * @brief Install the interaction on concrete interaction centres.
     * @param centres positions of interaction centres for a molecule in the default placement
     * @param pairData pair data used for all pairs of interaction centre types
     */
    void installOnCentres(const std::vector<Vector<3>> &centres, const PairData &pairData) {
        Expects(!centres.empty());

        this->potentialCentres = centres;
        this->centreIdxTypeMap = std::vector<std::size_t>(centres.size(), 0);
        this->pairDataMap = CentrePairDataMap<PairData>(1);
        this->pairDataMap.setPairData(0, 0, pairData);
    }

    /**
     * @brief Installs the interaction on concrete interaction centres with explicit interaction centre types.
     * @param centres positions of interaction centres for a molecule in the default placement
     * @param centreIdxTypeMap mapping from interaction centre indices to interaction centre types
     * @param pairDataMap map of pair data for all pairs of interaction centre types
     */
    void installOnCentres(const std::vector<Vector<3>> &centres, const std::vector<std::size_t> &centreIdxTypeMap,
                          const CentrePairDataMap<PairData> &pairDataMap) {
        Expects(!centres.empty());
        Expects(centres.size() == centreIdxTypeMap.size());
        Expects(*std::max_element(centreIdxTypeMap.begin(), centreIdxTypeMap.end()) < pairDataMap.getNumCentres());

        this->potentialCentres = centres;
        this->centreIdxTypeMap = centreIdxTypeMap;
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
