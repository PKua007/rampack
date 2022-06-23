//
// Created by Piotr Kubala on 04/01/2021.
//

#include "Interaction.h"

double Interaction::calculateEnergyBetweenShapes(const Shape &shape1, const Shape &shape2,
                                                 const BoundaryConditions &bc) const
{
    double energy{};

    auto centres = this->getInteractionCentres();
    if (centres.empty()) {
        return this->calculateEnergyBetween(shape1.getPosition(), shape1.getOrientation(), 0,
                                            shape2.getPosition(), shape2.getOrientation(), 0,
                                            bc);
    } else {
        for (std::size_t i{}; i < centres.size(); i++) {
            auto pos1 = Interaction::getCentrePositionForShape(shape1, centres[i]);
            for (std::size_t j{}; j < centres.size(); j++) {
                auto pos2 = Interaction::getCentrePositionForShape(shape2, centres[j]);
                energy += this->calculateEnergyBetween(pos1, shape1.getOrientation(), i,
                                                       pos2, shape2.getOrientation(), j,
                                                       bc);
            }
        }
    }

    return energy;
}

bool Interaction::overlapBetweenShapes(const Shape &shape1, const Shape &shape2, const BoundaryConditions &bc) const {
    auto centres = this->getInteractionCentres();
    if (centres.empty()) {
        return this->overlapBetween(shape1.getPosition(), shape1.getOrientation(), 0,
                                    shape2.getPosition(), shape2.getOrientation(), 0,
                                    bc);
    } else {
        for (std::size_t i{}; i < centres.size(); i++) {
            auto pos1 = Interaction::getCentrePositionForShape(shape1, centres[i]);
            for (std::size_t j{}; j < centres.size(); j++) {
                auto pos2 = Interaction::getCentrePositionForShape(shape2, centres[j]);
                if (this->overlapBetween(pos1, shape1.getOrientation(), i, pos2, shape2.getOrientation(), j, bc))
                    return true;
            }
        }
        return false;
    }
}

Vector<3> Interaction::getCentrePositionForShape(const Shape &shape, const Vector<3> &centre) {
    return shape.getPosition() + shape.getOrientation() * centre;
}

double Interaction::getTotalRangeRadius() const {
    auto centres = this->getInteractionCentres();
    double range = this->getRangeRadius();

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
    auto centres = this->getInteractionCentres();
    if (centres.empty()) {
        return this->overlapWithWall(shape.getPosition(), shape.getOrientation(), 0, wallOrigin, wallVector);
    } else {
        for (std::size_t i{}; i < centres.size(); i++) {
            auto pos = Interaction::getCentrePositionForShape(shape, centres[i]);
            if (this->overlapWithWall(pos, shape.getOrientation(), i, wallOrigin, wallVector))
                return true;
        }
        return false;
    }
}
