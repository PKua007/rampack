//
// Created by pkua on 05.03.2022.
//

#include "CompoundInteraction.h"
#include "utils/Exceptions.h"


CompoundInteraction::CompoundInteraction(const Interaction &interaction1, const Interaction &interaction2)
        : interaction1{interaction1}, interaction2{interaction2}
{
    auto centres1 = this->interaction1.getInteractionCentres();
    auto centres2 = this->interaction2.getInteractionCentres();
    Expects(centres1 == centres2);

    this->interactionCentres = centres1;
    this->hasHardPart1 = this->interaction1.hasHardPart();
    this->hasHardPart2 = this->interaction2.hasHardPart();
    this->hasSoftPart1 = this->interaction1.hasSoftPart();
    this->hasSoftPart2 = this->interaction2.hasSoftPart();
    this->hasWallPart1 = this->interaction1.hasWallPart();
    this->hasWallPart2 = this->interaction2.hasWallPart();

    bool isConvex1 = this->interaction1.isConvex();
    bool isConvex2 = this->interaction2.isConvex();
    if (this->hasHardPart1 && this->hasHardPart2)
        this->isThisConvex = false;
    else if ((this->hasHardPart1 && isConvex1) || (this->hasHardPart2 && isConvex2))
        this->isThisConvex = true;
    else
        this->isThisConvex = false;

    this->rangeRadius = std::max(this->interaction1.getRangeRadius(), this->interaction2.getRangeRadius());
    this->totalRangeRadius = std::max(this->interaction1.getTotalRangeRadius(),
                                      this->interaction2.getTotalRangeRadius());
}

double CompoundInteraction::calculateEnergyBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                                   std::size_t idx1, const Vector<3> &pos2,
                                                   const Matrix<3, 3> &orientation2, std::size_t idx2,
                                                   const BoundaryConditions &bc) const
{
    double energy{};
    if (this->hasSoftPart1)
        energy += this->interaction1.calculateEnergyBetween(pos1, orientation1, idx1, pos2, orientation2, idx2, bc);
    if (this->hasSoftPart2)
        energy += this->interaction2.calculateEnergyBetween(pos1, orientation1, idx1, pos2, orientation2, idx2, bc);
    return energy;
}

bool CompoundInteraction::overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1, std::size_t idx1,
                                         const Vector<3> &pos2, const Matrix<3, 3> &orientation2, std::size_t idx2,
                                         const BoundaryConditions &bc) const
{
    if (this->hasHardPart1 && this->interaction1.overlapBetween(pos1, orientation1, idx1, pos2, orientation2, idx2, bc))
        return true;
    if (this->hasHardPart2 && this->interaction2.overlapBetween(pos1, orientation1, idx1, pos2, orientation2, idx2, bc))
        return true;
    return false;
}

bool CompoundInteraction::overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, std::size_t idx,
                                          const Vector<3> &wallOrigin,  const Vector<3> &wallVector) const
{
    if (this->hasWallPart1 && this->interaction1.overlapWithWall(pos, orientation, idx, wallOrigin, wallVector))
        return true;
    if (this->hasWallPart2 && this->interaction2.overlapWithWall(pos, orientation, idx, wallOrigin, wallVector))
        return true;
    return false;
}
