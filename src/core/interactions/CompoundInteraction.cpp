//
// Created by pkua on 05.03.2022.
//

#include "CompoundInteraction.h"
#include "utils/Exceptions.h"


CompoundInteraction::CompoundInteraction(const Interaction &interaction1, const Interaction &interaction2)
        : interaction1{interaction1}, interaction2{interaction2}
{
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
}

double CompoundInteraction::calculateEnergyBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                                   const std::byte *data1, std::size_t idx1, const Vector<3> &pos2,
                                                   const Matrix<3, 3> &orientation2, const std::byte *data2,
                                                   std::size_t idx2, const BoundaryConditions &bc) const
{
    double energy{};
    if (this->hasSoftPart1) {
        energy += this->interaction1.calculateEnergyBetween(pos1, orientation1, data1, idx1,
                                                            pos2, orientation2, data2, idx2,
                                                            bc);
    }
    if (this->hasSoftPart2) {
        energy += this->interaction2.calculateEnergyBetween(pos1, orientation1, data1, idx1,
                                                            pos2, orientation2, data2, idx2,
                                                            bc);
    }
    return energy;
}

bool CompoundInteraction::overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                         const std::byte *data1, std::size_t idx1, const Vector<3> &pos2,
                                         const Matrix<3, 3> &orientation2, const std::byte *data2, std::size_t idx2,
                                         const BoundaryConditions &bc) const
{
    if (this->hasHardPart1 && this->interaction1.overlapBetween(pos1, orientation1, data1, idx1,
                                                                pos2, orientation2, data2, idx2,
                                                                bc))
    {
        return true;
    }
    if (this->hasHardPart2 && this->interaction2.overlapBetween(pos1, orientation1, data1, idx1,
                                                                pos2, orientation2, data2, idx2,
                                                                bc))
    {
        return true;
    }
    return false;
}

bool CompoundInteraction::overlapWithWall(const Vector<3> &pos, const Matrix<3, 3> &orientation, const std::byte *data,
                                          std::size_t idx, const Vector<3> &wallOrigin,
                                          const Vector<3> &wallVector) const
{
    if (this->hasWallPart1
        && this->interaction1.overlapWithWall(pos, orientation, data, idx, wallOrigin, wallVector))
    {
        return true;
    }
    if (this->hasWallPart2
        && this->interaction2.overlapWithWall(pos, orientation, data, idx, wallOrigin, wallVector))
    {
        return true;
    }
    return false;
}

std::vector<Vector<3>> CompoundInteraction::getInteractionCentres(const std::byte *data) const {
    auto centres1 = this->interaction1.getInteractionCentres(data);
    auto centres2 = this->interaction2.getInteractionCentres(data);
    Assert(centres1 == centres2);

    return centres1;
}
