//
// Created by Piotr Kubala on 04/01/2021.
//

#include "Interaction.h"
#include "utils/Utils.h"


double Interaction::calculateEnergyBetweenShapes(const Shape &shape1, const Shape &shape2,
                                                 const BoundaryConditions &bc) const
{
    auto centres1 = this->getInteractionCentres(shape1.getData().raw());
    auto centres2 = this->getInteractionCentres(shape2.getData().raw());
    Expects(logical_xnor(centres1.empty(), centres2.empty()));

    double energy{};

    if (centres1.empty()) {
        return this->calculateEnergyBetween(shape1.getPosition(), shape1.getOrientation(), shape1.getData().raw(), 0,
                                            shape2.getPosition(), shape2.getOrientation(), shape2.getData().raw(), 0,
                                            bc);
    } else {
        for (std::size_t i{}; i < centres1.size(); i++) {
            auto pos1 = Interaction::getCentrePositionForShape(shape1, centres1[i]);
            for (std::size_t j{}; j < centres2.size(); j++) {
                auto pos2 = Interaction::getCentrePositionForShape(shape2, centres2[j]);
                energy += this->calculateEnergyBetween(pos1, shape1.getOrientation(), shape1.getData().raw(), i,
                                                       pos2, shape2.getOrientation(), shape2.getData().raw(), j,
                                                       bc);
            }
        }
    }

    return energy;
}

bool Interaction::overlapBetweenShapes(const Shape &shape1, const Shape &shape2, const BoundaryConditions &bc) const {
    auto centres1 = this->getInteractionCentres(shape1.getData().raw());
    auto centres2 = this->getInteractionCentres(shape2.getData().raw());
    Expects(logical_xnor(centres1.empty(), centres2.empty()));

    if (centres1.empty()) {
        return this->overlapBetween(shape1.getPosition(), shape1.getOrientation(), shape1.getData().raw(), 0,
                                    shape2.getPosition(), shape2.getOrientation(), shape2.getData().raw(), 0,
                                    bc);
    } else {
        for (std::size_t i{}; i < centres1.size(); i++) {
            auto pos1 = Interaction::getCentrePositionForShape(shape1, centres1[i]);
            for (std::size_t j{}; j < centres2.size(); j++) {
                auto pos2 = Interaction::getCentrePositionForShape(shape2, centres2[j]);
                if (this->overlapBetween(pos1, shape1.getOrientation(), shape1.getData().raw(), i,
                                         pos2, shape2.getOrientation(), shape2.getData().raw(), j,
                                         bc))
                {
                    return true;
                }
            }
        }
        return false;
    }
}

Vector<3> Interaction::getCentrePositionForShape(const Shape &shape, const Vector<3> &centre) {
    return shape.getPosition() + shape.getOrientation() * centre;
}

double Interaction::getTotalRangeRadius(const std::byte *data) const {
    auto centres = this->getInteractionCentres(nullptr);
    double range = this->getRangeRadius(data);

    if (centres.empty())
        return range;

    double maxRadius2 = 0;
    for (const auto &centre : centres) {
        double radius2 = centre.norm2();
        if (radius2 > maxRadius2)
            maxRadius2 = radius2;
    }

    return 2*std::sqrt(maxRadius2) + range;
}

bool Interaction::overlapWithWallForShape(const Shape &shape, const Vector<3> &wallOrigin,
                                          const Vector<3> &wallVector) const
{
    auto centres = this->getInteractionCentres(shape.getData().raw());
    if (centres.empty()) {
        return this->overlapWithWall(shape.getPosition(), shape.getOrientation(), shape.getData().raw(), 0, wallOrigin,
                                     wallVector);
    } else {
        for (std::size_t i{}; i < centres.size(); i++) {
            auto pos = Interaction::getCentrePositionForShape(shape, centres[i]);
            if (this->overlapWithWall(pos, shape.getOrientation(), shape.getData().raw(), i, wallOrigin, wallVector))
                return true;
        }
        return false;
    }
}
