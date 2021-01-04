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

double Interaction::overlapBetweenShapes(const Shape &shape1, const Shape &shape2, const BoundaryConditions &bc) const {
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
