//
// Created by pkua on 05.03.2022.
//

#ifndef RAMPACK_COMPOUNDINTERACTION_H
#define RAMPACK_COMPOUNDINTERACTION_H

#include "core/Interaction.h"


class CompoundInteraction : public Interaction {
private:
    const Interaction &interaction1;
    const Interaction &interaction2;

    std::vector<Vector<3>> interactionCentres;
    double rangeRadius;
    double totalRangeRadius;
    bool hasSoftPart1;
    bool hasSoftPart2;
    bool hasHardPart1;
    bool hasHardPart2;

public:
    CompoundInteraction(const Interaction &interaction1, const Interaction &interaction2);

    [[nodiscard]] bool hasHardPart() const override { return this->hasHardPart1 || this->hasHardPart2; }
    [[nodiscard]] bool hasSoftPart() const override { return this->hasSoftPart1 || this->hasSoftPart2; }
    [[nodiscard]] double getRangeRadius() const override { return this->rangeRadius; }
    [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const override { return this->interactionCentres; }
    [[nodiscard]] double getTotalRangeRadius() const override { return this->totalRangeRadius; }

    [[nodiscard]] double calculateEnergyBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                                std::size_t idx1, const Vector<3> &pos2,
                                                const Matrix<3, 3> &orientation2, std::size_t idx2,
                                                const BoundaryConditions &bc) const override;

    [[nodiscard]] bool overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                      const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                      const BoundaryConditions &bc) const override;
};


#endif //RAMPACK_COMPOUNDINTERACTION_H
