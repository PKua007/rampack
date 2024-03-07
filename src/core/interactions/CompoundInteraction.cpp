//
// Created by pkua on 05.03.2022.
//

#include "CompoundInteraction.h"

#include <utility>
#include "utils/Exceptions.h"


CompoundInteraction::CompoundInteraction(const Interaction &mainInteraction, const Interaction &helperInteraction,
                                         ShapeData helperData)
        : mainInteration{mainInteraction}, helperInteration{helperInteraction}, helperData{std::move(helperData)}
{
    this->hasHardPartMain = this->mainInteration.hasHardPart();
    this->hasHardPartHelper = this->helperInteration.hasHardPart();
    this->hasSoftPartMain = this->mainInteration.hasSoftPart();
    this->hasSoftPartHelper = this->helperInteration.hasSoftPart();
    this->hasWallPartMain = this->mainInteration.hasWallPart();
    this->hasWallPartHelper = this->helperInteration.hasWallPart();
    this->helperRadius = this->helperInteration.getRangeRadius(this->helperData.raw());
    this->helperTotalRadius = this->helperInteration.getTotalRangeRadius(this->helperData.raw());
    this->helperInteractionCentres = this->helperInteration.getInteractionCentres(this->helperData.raw());

    bool isConvexMain = this->mainInteration.isConvex();
    bool isConvexHelper = this->helperInteration.isConvex();
    if (this->hasHardPartMain && this->hasHardPartHelper)
        this->isThisConvex = false;
    else if ((this->hasHardPartMain && isConvexMain) || (this->hasHardPartHelper && isConvexHelper))
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
    if (this->hasSoftPartMain) {
        energy += this->mainInteration.calculateEnergyBetween(pos1, orientation1, data1, idx1,
                                                              pos2, orientation2, data2, idx2,
                                                              bc);
    }
    if (this->hasSoftPartHelper) {
        energy += this->helperInteration.calculateEnergyBetween(pos1, orientation1, this->helperData.raw(), idx1,
                                                                pos2, orientation2, this->helperData.raw(), idx2,
                                                                bc);
    }
    return energy;
}

bool CompoundInteraction::overlapBetween(const Vector<3> &pos1, const Matrix<3, 3> &orientation1,
                                         const std::byte *data1, std::size_t idx1, const Vector<3> &pos2,
                                         const Matrix<3, 3> &orientation2, const std::byte *data2, std::size_t idx2,
                                         const BoundaryConditions &bc) const
{
    if (this->hasHardPartMain
        && this->mainInteration.overlapBetween(pos1, orientation1, data1, idx1, pos2, orientation2, data2, idx2, bc))
    {
        return true;
    }
    if (this->hasHardPartHelper
        && this->helperInteration.overlapBetween(pos1, orientation1, this->helperData.raw(), idx1,
                                                 pos2, orientation2, this->helperData.raw(), idx2,
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
    if (this->hasWallPartMain
        && this->mainInteration.overlapWithWall(pos, orientation, data, idx, wallOrigin, wallVector))
    {
        return true;
    }
    if (this->hasWallPartHelper
        && this->helperInteration.overlapWithWall(pos, orientation, this->helperData.raw(), idx, wallOrigin,
                                                  wallVector))
    {
        return true;
    }
    return false;
}

std::vector<Vector<3>> CompoundInteraction::getInteractionCentres(const std::byte *data) const {
    auto centres1 = this->mainInteration.getInteractionCentres(data);
    AssertMsg(centres1 == this->helperInteractionCentres, "Non identical centres for given data");

    return centres1;
}
