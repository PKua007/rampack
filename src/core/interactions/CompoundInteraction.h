//
// Created by pkua on 05.03.2022.
//

#ifndef RAMPACK_COMPOUNDINTERACTION_H
#define RAMPACK_COMPOUNDINTERACTION_H

#include "core/Interaction.h"

/**
 * @brief The class joining two interaction into one Interaction class.
 * @details Both soft and hard parts are correctly combined.
 */
class CompoundInteraction : public Interaction {
private:
    const Interaction &mainInteration;
    const Interaction &helperInteration;

    bool hasSoftPartMain{};
    bool hasHardPartMain{};
    bool hasWallPartMain{};

    bool hasSoftPartHelper{};
    bool hasHardPartHelper{};
    bool hasWallPartHelper{};
    ShapeData helperData;
    double helperRadius{};
    double helperTotalRadius{};
    std::vector<Vector<3>> helperInteractionCentres;

    bool isThisConvex{};

public:
    /**
     * @brief Creates the object for two given constituent interactions.
     * @details Both interaction have to have identical interaction centres, otherwise an exception is thrown.
     */
    CompoundInteraction(const Interaction &mainInteraction, const Interaction &helperInteraction,
                        ShapeData helperData = {});

    [[nodiscard]] bool hasHardPart() const override { return this->hasHardPartMain || this->hasHardPartHelper; }
    [[nodiscard]] bool hasSoftPart() const override { return this->hasSoftPartMain || this->hasSoftPartHelper; }
    [[nodiscard]] bool hasWallPart() const override { return this->hasWallPartMain || this->hasWallPartHelper; }
    [[nodiscard]] bool isConvex() const override { return this->isThisConvex; }

    [[nodiscard]] double getRangeRadius(const std::byte *data) const override {
        return std::max(this->mainInteration.getRangeRadius(data), this->helperRadius);
    }

    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres(const std::byte *data) const override;

    [[nodiscard]] double getTotalRangeRadius(const std::byte *data) const override {
        return std::max(this->mainInteration.getTotalRangeRadius(data), this->helperTotalRadius);
    }

    [[nodiscard]] double calculateEnergyBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                                const std::byte *data1, std::size_t idx1, const Vector<3> &pos2,
                                                const Matrix<3, 3> &orientation2, const std::byte *data2,
                                                std::size_t idx2, const BoundaryConditions &bc) const override;

    [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, const std::byte *data1,
                                      std::size_t idx1, const Vector<3> &pos2, const Matrix<3, 3> &orientation2,
                                      const std::byte *data2, std::size_t idx2,
                                      const BoundaryConditions &bc) const override;

    [[nodiscard]] bool overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, const std::byte *data,
                                       std::size_t idx, const Vector<3> &wallOrigin,
                                       const Vector<3> &wallVector) const override;
};


#endif //RAMPACK_COMPOUNDINTERACTION_H
